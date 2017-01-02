#include "echo.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

Echo::Echo(Processor* processor) {
    // Store and compute useful information
    sampleRate = processor->sampleRate;
    windowSize = processor->windowSize;
    overlapFactor = processor->overlapFactor;
    amountOfBins = processor->windowSize/2 + 1;
    freqCoefficients = processor->frequencyCoefficients;

    echoTime = 0.5;
    echoFactor = 0.9;
    
    length = sampleRate / windowSize * overlapFactor * echoTime;
    
    // Create buffers
    freqCoefficientsBuffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * amountOfBins * length);
    memset(freqCoefficientsBuffer, 0, sizeof(fftw_complex) * amountOfBins * length);
    
    // Set (default) name and options
    strcpy(name, "Echo");
    amountOfOptions = 2;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("time", 0));
    options[0] = 0.0;
    
    optionLabels.push_back(OptionLabel("factor", 1));
    options[1] = 0.0;
}

int Echo::apply() {
    // Declare variables
    unsigned int k;
    
    for(k = 0;k < amountOfBins;k ++) {
        freqCoefficients[k][0] += echoFactor * freqCoefficientsBuffer[k][0];
        freqCoefficients[k][1] += echoFactor * freqCoefficientsBuffer[k][1];
    }
    
    memmove(freqCoefficientsBuffer, freqCoefficientsBuffer + amountOfBins, sizeof(fftw_complex) * amountOfBins * (length - 1));
    
    for(k = 0;k < amountOfBins;k ++) {
        freqCoefficientsBuffer[amountOfBins*(length - 1) + k][0] = freqCoefficients[k][0];
        freqCoefficientsBuffer[amountOfBins*(length - 1) + k][1] = freqCoefficients[k][1];
    }
    
    return 1;
}

Echo::~Echo() {
    // Deallocate memory
    fftw_free(freqCoefficientsBuffer);
    delete[] options;
}
