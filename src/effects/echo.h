#ifndef ECHO_H
#define ECHO_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Echo : public Effect {
    
    double sampleRate;
    int windowSize;
    int overlapFactor;
    int amountOfBins;
    
    double echoTime;
    double echoFactor;
    int length;
    
    fftw_complex* freqCoefficients;
    fftw_complex* freqCoefficientsBuffer;
    
public:
    
    Echo(Processor*);
    ~Echo();
    int apply();
    
};

#endif
