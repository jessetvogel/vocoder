#ifndef NOISEFILTER_H
#define NOISEFILTER_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class NoiseFilter : public Effect {
    
    int amountOfBins;
    
    fftw_complex* freqCoefficients;
    
public:

    NoiseFilter(Processor*);
    ~NoiseFilter();
    int apply();
    
};

#endif
