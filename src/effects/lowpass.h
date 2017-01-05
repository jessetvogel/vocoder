#ifndef LOWPASS_H
#define LOWPASS_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class LowPass : public Effect {
    
    int amountOfBins;
    int amountOfChannels;
    double freqPerBin;
    
    double centerFrequency;
    
    fftw_complex* freqCoefficients;
    
public:
    
    LowPass(Processor*);
    ~LowPass();
    int apply();
    int applyToChannel(unsigned int);
    
};

#endif
