#include "noisefilter.h"
#include <cmath>

NoiseFilter::NoiseFilter(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    
    // Set (default) name and options
    strcpy(name, "NoiseFilter");
    amountOfOptions = 1;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("threshold", 0));
    options[0] = 0.0;
}

int NoiseFilter::apply() {
    // Declare variables
    double magn, real, imag, totalMagn = 0.0;
    unsigned int k, channel;
    fftw_complex* _freqCoefficients;
    
    // For every bin, if the magnitude is below a certain threshold, set it to zero
    double threshold = options[0];
    for(channel = 0;channel < amountOfChannels;channel ++) {
        _freqCoefficients = freqCoefficients + channel * amountOfBins;
        for(k = 0;k < amountOfBins;k ++) {
            // Compute magnitude and phase of each bin
            real = _freqCoefficients[k][0];
            imag = _freqCoefficients[k][1];
            
            magn = sqrt(real*real + imag*imag);
            totalMagn += magn;
            
            if(magn < threshold) {
                _freqCoefficients[k][0] = 0.0;
                _freqCoefficients[k][1] = 0.0;
            }
        }
    }
    
    return 1;
}

NoiseFilter::~NoiseFilter()  {
    // Deallocate memory
    delete[] options;
}