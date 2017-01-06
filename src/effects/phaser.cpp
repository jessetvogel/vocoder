#include "phaser.h"
#include <cmath>

Phaser::Phaser(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    freqPerBin = processor->sampleRate / processor->windowSize;
    
    // Set (default) name and options
    strcpy(name, "Phaser");
    amountOfOptions = 2;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("stages", 0));
    options[0] = 0.0;
    optionLabels.push_back(OptionLabel("depth", 1));
    options[1] = 0.0;
}

int z = 0;

int Phaser::apply() {
    // Apply to all channels
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    z ++;
    
    return 1;
}

int Phaser::applyToChannel(unsigned int channel) {
    // Declare variables
    double real, imag, magn, phase;
    unsigned int k;
    fftw_complex* _freqCoefficients = freqCoefficients + channel * amountOfBins;
    
    // For every bin, multiply its value by gain
    for(k = 0;k < amountOfBins;k ++) {
        real = _freqCoefficients[k][0];
        imag = _freqCoefficients[k][1];
        
        magn = sqrt(real*real + imag*imag);
        phase = atan2(imag, real);
        
        double freq = 200.0 + 100.0 * sin((double) z / 10.0);
        phase -= atan((double) k * freqPerBin / freq) * options[0];
        
        // Compute magnitude and phase of each bin
        _freqCoefficients[k][0] += magn * cos(phase) * options[1];
        _freqCoefficients[k][1] += magn * sin(phase) * options[1];
    }
    
    return 1;
}

Phaser::~Phaser() {
    // Deallocate memory
    delete[] options;
}
