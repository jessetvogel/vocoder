#ifndef PHASER_H
#define PHASER_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Phaser : public Effect {
    
    int amountOfBins;
    int amountOfChannels;
    double freqPerBin;

    fftw_complex* freqCoefficients;
    
public:
    
    Phaser(Processor*);
    ~Phaser();
    int apply();
    int applyToChannel(unsigned int);
    
};

#endif
