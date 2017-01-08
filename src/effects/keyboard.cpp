#include "keyboard.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "../vocoderUtil/util.h"

#include <iostream>

Keyboard::Keyboard(Processor* processor, int amountOfTones) {
    // Store and compute useful information
    sampleRate = processor->sampleRate;
    windowSize = processor->windowSize;
    overlapFactor = processor->overlapFactor;
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    freqPerBin = sampleRate / processor->windowSize;
    expectedPhase = 2.0 * M_PI / overlapFactor;
    this->amountOfTones = amountOfTones;

    // Create buffers
    lastPhase = new double[amountOfBins * amountOfChannels];
    sumPhase = new double[amountOfBins * amountOfTones * amountOfChannels];
    analyticMagn = new double[amountOfBins];
    analyticFreq = new double[amountOfBins];
    syntheticMagn = new double[amountOfBins];
    syntheticFreq = new double[amountOfBins];
    
    memset(lastPhase, 0, sizeof(double) * amountOfBins * amountOfChannels);
    memset(sumPhase, 0, sizeof(double) * amountOfBins * amountOfTones * amountOfChannels);
    memset(analyticMagn, 0, sizeof(double) * amountOfBins);
    memset(analyticFreq, 0, sizeof(double) * amountOfBins);
    memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
    memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
    
    // Set (default) name and options
    strcpy(name, "Keyboard");
    amountOfOptions = 2 * amountOfTones;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("tones", 0));
    
    for(unsigned int t = 0;t < amountOfTones;t ++) {
        options[2*t] = 0.0; // Frequency
        options[2*t+1] = 0.0; // Magnitude
    }
}

int Keyboard::apply() {
    // Apply to all channels
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    return 1;
}

int Keyboard::applyToChannel(unsigned int channel) {
    // Declare variables
    fftw_complex* _freqCoefficients = freqCoefficients + channel * amountOfBins;
    double magn, phase, tmp, real, imag;
    unsigned int t, k, index;
    
    // Analysis
    util::analysis(_freqCoefficients, lastPhase + channel * amountOfBins, amountOfBins, overlapFactor, freqPerBin, analyticMagn, analyticFreq);
    
    // Determine fundamental frequency if first channel
    if(channel == 0) {
        tmp = util::fundamentalFrequency(analyticMagn, analyticFreq, amountOfBins, freqPerBin);
        util::certainty = 0.1 + 0.9 * util::certainty;
        fundamentalFreq = util::certainty * tmp + (1.0 - util::certainty) * fundamentalFreq;
    }
    
    // In case of silence, do nothing
    if(fundamentalFreq <= 0.0) return 1;
    
    // Processing
    memset(_freqCoefficients, 0, sizeof(fftw_complex) * amountOfBins);
    for(t = 0;t < amountOfTones;t ++) {
        // If tone has no additional value, neglect it
        if(options[2*t + 1] == 0) continue;
            
        memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
        memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
        double pitchShift = options[2*t] / fundamentalFreq;
        for(k = 0; k < amountOfBins;k ++) {
            index = round(k * pitchShift);
            if (index < amountOfBins) {
                syntheticMagn[index] += analyticMagn[k] * analyticMagn[k] * options[2*t + 1];
                syntheticFreq[index] = analyticFreq[k] * pitchShift;
            }
        }
        
        for(index = 0;index < amountOfBins;index ++) {
            syntheticMagn[index] = sqrt(syntheticMagn[index]);
        }
        
        // Partial synthesis
        util::synthesisAdd(_freqCoefficients, sumPhase + amountOfBins * (t + amountOfTones * channel), amountOfBins, overlapFactor, freqPerBin, syntheticMagn, syntheticFreq);
    }
    
    return 1;
}

Keyboard::~Keyboard() {
    // Deallocate memory TODO: check in all other files if also done "delete[]" correctly
    delete[] lastPhase;
    delete[] sumPhase;
    delete[] analyticMagn;
    delete[] analyticFreq;
    delete[] syntheticMagn;
    delete[] syntheticFreq;
    delete[] options;
}
