#include "equalizer.h"
#include <cmath>

Equalizer::Equalizer(Processor* processor, double frequencyMin, double frequencyMax, int N) {
    // Store and compute useful information
    sampleRate = processor->sampleRate;
    freqCoefficients = processor->frequencyCoefficients;
    freqPerBin = sampleRate / processor->windowSize;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    
    this->frequencyMin = frequencyMin;
    this->frequencyMax = frequencyMax;
    this->N = N;
    logBase = 1.0 / log2(frequencyMax / frequencyMin);
    
    // Set (default) name and options
    strcpy(name, "Equalizer");
    amountOfOptions = N;
    options = new double[amountOfOptions];

    optionLabels.push_back(OptionLabel("values", 0));
    for(unsigned int i = 0;i < N;i ++) {
        options[i] = 1.0;
    }
}

int Equalizer::apply() {
    // Declare variables
    unsigned int k;
    double gain, gradient;
    
    // For every bin, multiply its value by a 'gradiented' gain
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude and phase of each bin
        double x = log2(freqPerBin * k / frequencyMin) * logBase * (N - 1);
        if(x <= 0) gain = options[0];
        else if(x >= (double) (N - 1)) gain = options[N - 1];
        else {
            int before = floor(x);
            gradient = x - before;
            gain = (1.0 - gradient) * options[before] + gradient * options[before + 1];
        }
        
        for(unsigned int channel = 0;channel < amountOfChannels;channel ++) {
            freqCoefficients[k + channel * amountOfBins][0] *= gain;
            freqCoefficients[k + channel * amountOfBins][1] *= gain;
        }
    }
    
    return 1;
}

Equalizer::~Equalizer() {
    // Deallocate memory
    delete[] options;
}