#ifndef REVERB_H
#define REVERB_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Reverb : public Effect {
    
    int amountOfBins;
    int amountOfChannels;
    
    double expectedPhase;
    
    fftw_complex* freqCoefficients;
    fftw_complex* freqCoefficientsBuffer;
    
public:
    
    Reverb(Processor*);
    ~Reverb();
    int apply();
    int applyToChannel(unsigned int);
    
};

#endif
