#ifndef PAN_H
#define PAN_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Pan : public Effect {
    
    int amountOfBins;
    
    fftw_complex* freqCoefficients;
    
public:
    
    Pan(Processor*);
    int apply();
    
};

#endif
