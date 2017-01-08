#include "pitch.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "../vocoderUtil/util.h"

Pitch::Pitch(Processor* processor) {
    // Store and compute useful information
    sampleRate = processor->sampleRate;
    windowSize = processor->windowSize;
    overlapFactor = processor->overlapFactor;
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    freqPerBin = sampleRate / processor->windowSize;
    expectedPhase = 2.0 * M_PI / overlapFactor;
    
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
    strcpy(name, "Pitch");
    amountOfOptions = 1;
    options = new double[amountOfOptions];

    optionLabels.push_back(OptionLabel("shift", 0)); options[0] = 1.0;
}

int Pitch::apply() {
    // Fix pitchShift (in case of asynchronous setting of options)
    pitchShift = options[0];
    
    // Apply a pitch shift to all channels
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    return 1;
}

int Pitch::applyToChannel(unsigned int channel) {
    int offset  = channel * amountOfBins;
    fftw_complex* _freqCoefficients = freqCoefficients + offset;
    
    // Declare variables
    double magn, phase, tmp, real, imag;
    unsigned int k, index;

    // Step 1: Analysis
    util::analysis(_freqCoefficients, lastPhase + offset, amountOfBins, overlapFactor, freqPerBin, analyticMagn, analyticFreq);
    
    // Step 2: Processing
    memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
    memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
    for(k = 0; k < amountOfBins;k ++) {
        index = round(k * pitchShift);
        if (index < amountOfBins) {
            syntheticMagn[index] += analyticMagn[k] * analyticMagn[k];
            syntheticFreq[index] = analyticFreq[k] * pitchShift;
        }
    }
    
    for(index = 0;index < amountOfBins;index ++) {
        syntheticMagn[index] = sqrt(syntheticMagn[index]);
    }
    
    // Step 3: Synthesis
    util::synthesis(_freqCoefficients, sumPhase + offset, amountOfBins, overlapFactor, freqPerBin, syntheticMagn, syntheticFreq);
    
    return 1;
}

Pitch::~Pitch() {
    // Deallocate memory TODO: check in all other files if also done "delete[]" correctly
    delete[] lastPhase;
    delete[] sumPhase;
    delete[] analyticMagn;
    delete[] analyticFreq;
    delete[] syntheticMagn;
    delete[] syntheticFreq;
    delete[] options;
}
