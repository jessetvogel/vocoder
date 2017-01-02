#ifndef GAIN_H
#define GAIN_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Gain : public Effect {
    
    int amountOfBins;
    int amountOfChannels;
    
    double gain;
    
    fftw_complex* freqCoefficients;
    
public:
    
    Gain(Processor*);
    ~Gain();
    int apply();
    int applyToChannel(unsigned int);
    
};

#endif
