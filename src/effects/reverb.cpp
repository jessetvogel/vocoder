#include "reverb.h"
#include <cmath>

Reverb::Reverb(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    expectedPhase = 2.0 * M_PI / processor->overlapFactor;
    
    // Create buffers
    freqCoefficientsBuffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * amountOfBins * amountOfChannels);
    memset(freqCoefficientsBuffer, 0, sizeof(fftw_complex) * amountOfBins * amountOfChannels);
    
    // Set (default) name and options
    strcpy(name, "Reverb");
    amountOfOptions = 1;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("RT60", 0));
    options[0] = 0.0;
}

int Reverb::apply() {
    // Apply reverberation to all channels
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    // Copy coefficients to buffer for next frame
    memcpy(freqCoefficientsBuffer, freqCoefficients, sizeof(fftw_complex) * amountOfBins * amountOfChannels);
    
    return 1;
}

int Reverb::applyToChannel(unsigned int channel) {
    // Declare variables
    unsigned int k;
    double magn, phase, real, imag;
    fftw_complex* _freqCoefficients = freqCoefficients + channel * amountOfBins;
    fftw_complex* _freqCoefficientsBuffer = freqCoefficientsBuffer + channel * amountOfBins;
    
    double RT60 = options[0];
    double factor = RT60; // TODO 0.001^(tau / RT60)
    for(k = 0;k < amountOfBins;k ++) {
        real = _freqCoefficientsBuffer[k][0];
        imag = _freqCoefficientsBuffer[k][1];
        
        magn = sqrt(real*real + imag*imag);
        phase = atan2(imag, real) + (double) k * expectedPhase;
        
        _freqCoefficients[k][0] += magn * cos(phase) * factor;
        _freqCoefficients[k][1] += magn * sin(phase) * factor;
    }
    
    return 1;
}

Reverb::~Reverb() {
    // Deallocate memory
    delete[] options;
    fftw_free(freqCoefficientsBuffer);
}