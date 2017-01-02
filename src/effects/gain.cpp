#include "gain.h"
#include <cmath>

Gain::Gain(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    
    // Set (default) name and options
    strcpy(name, "Gain");
    amountOfOptions = 1;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("gain", 0));
    options[0] = 1.0;
}

int Gain::apply() {
    // Apply to all channels
    gain = options[0];
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    return 1;
}

int Gain::applyToChannel(unsigned int channel) {
    // Declare variables
    unsigned int k;
    fftw_complex* _freqCoefficients = freqCoefficients + channel * amountOfBins;
    
    // For every bin, multiply its value by gain
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude and phase of each bin
        _freqCoefficients[k][0] *= gain;
        _freqCoefficients[k][1] *= gain;
    }
    
    return 1;
}

Gain::~Gain() {
    // Deallocate memory
    delete[] options;
}