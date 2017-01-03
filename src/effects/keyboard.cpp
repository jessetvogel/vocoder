#include "keyboard.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "../vocoderUtil/util.h"

#include <iostream>

Keyboard::Keyboard(Processor* processor, unsigned int amountOfTones) {
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
    sumPhase = new double[amountOfBins * amountOfChannels];
    analyticMagn = new double[amountOfBins];
    analyticFreq = new double[amountOfBins];
    syntheticMagn = new double[amountOfBins];
    syntheticFreq = new double[amountOfBins];
    
    memset(lastPhase, 0, sizeof(double) * amountOfBins * amountOfChannels);
    memset(sumPhase, 0, sizeof(double) * amountOfBins * amountOfChannels);
    memset(analyticMagn, 0, sizeof(double) * amountOfBins);
    memset(analyticFreq, 0, sizeof(double) * amountOfBins);
    memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
    memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
    
    // Set (default) name and options
    strcpy(name, "Keyboard");
    amountOfOptions = amountOfTones * 2;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("tones", 0));
    for(unsigned int t = 0;t < amountOfTones;t ++) {
        options[2*t] = 300.0; // Frequency
        options[2*t+1] = 1.0; // Gain
    }
}

int Keyboard::apply() {
    // Apply a pitch shift to all channels
    fundamentalFreq = -1.0;
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    return 1;
}

int Keyboard::applyToChannel(unsigned int channel) {
    int offset  = channel * amountOfBins;
    fftw_complex* _freqCoefficients = freqCoefficients + offset;
    
    // Declare variables
    double magn, phase, tmp, real, imag, pitchShift, maxMagn = 0.0, maxFreq = 1.0;
    unsigned int k, t, index;
    
    // Step 1: Analysis
    util::analysis(_freqCoefficients, lastPhase + offset, amountOfBins, overlapFactor, freqPerBin, analyticMagn, analyticFreq);
    
    // Step 2: Determine the fundamental frequency (this is only done for the first channel, the other channels will use this same frequency)
    if(fundamentalFreq < 0.0) {
        fundamentalFreq = util::fundamentalFrequency(analyticMagn, analyticFreq, amountOfBins, freqPerBin);
    }
    
    if(fundamentalFreq <= 0.0) {
        return 1;
    }
    
    // Step 3: Processing
    memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
    memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
    for(k = 0;k < amountOfBins;k ++) {
        for(t = 0;t < amountOfTones;t ++) {
            pitchShift = options[2*t] / fundamentalFreq;
            index = round(k * pitchShift);
            if (index < amountOfBins) {
                syntheticMagn[index] += analyticMagn[k] * options[2*t+1]; // TODO: fix asynchronous update of options stuff...
                syntheticFreq[index] = analyticFreq[k] * pitchShift;
            }
        }
    }
    
    // Step 4: Synthesis
    util::synthesis(_freqCoefficients, sumPhase + offset, amountOfBins, overlapFactor, freqPerBin, syntheticMagn, syntheticFreq);
    
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
