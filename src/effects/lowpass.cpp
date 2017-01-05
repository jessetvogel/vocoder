#include "lowpass.h"
#include <cmath>

LowPass::LowPass(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    freqPerBin = processor->sampleRate / processor->windowSize;

    // Set (default) name and options
    strcpy(name, "LowPass");
    amountOfOptions = 1;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("center", 0));
    options[0] = 1000.0;
}

int LowPass::apply() {
    // Apply to all channels
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    return 1;
}

int LowPass::applyToChannel(unsigned int channel) {
    // Declare variables
    double real, imag, magn, phase, tmp;
    unsigned int k;
    fftw_complex* _freqCoefficients = freqCoefficients + channel * amountOfBins;
    
    // For every bin, multiply its value by gain
    for(k = 0;k < amountOfBins;k ++) {
        // Adjust magn and phase according to a low pass filter
        real = _freqCoefficients[k][0];
        imag = _freqCoefficients[k][1];
 
        magn = sqrt(real*real + imag*imag);
        phase = atan2(imag, real);
        
        tmp = (double) k * freqPerBin / options[0];
        magn /= 1.0 + tmp;
        phase -= atan(tmp);
        
        _freqCoefficients[k][0] = magn * cos(phase);
        _freqCoefficients[k][1] = magn * sin(phase);
    }
    
    return 1;
}

LowPass::~LowPass() {
    // Deallocate memory
    delete[] options;
}
