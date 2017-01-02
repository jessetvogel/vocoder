#include "noisefilter.h"
#include <cmath>

NoiseFilter::NoiseFilter(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    
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
    unsigned int k;
    
    // For every bin, if the magnitude is below a certain threshold, set it to zero
    double threshold = options[0];
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude and phase of each bin
        real = freqCoefficients[k][0];
        imag = freqCoefficients[k][1];
        
        magn = sqrt(real*real + imag*imag);
        totalMagn += magn;
        
        if(magn < threshold) {
            freqCoefficients[k][0] = 0.0;
            freqCoefficients[k][1] = 0.0;
        }
    }
    
    return 1;
}

NoiseFilter::~NoiseFilter()  {
    // Deallocate memory
    delete[] options;
}