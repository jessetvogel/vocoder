#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Keyboard : public Effect {
    
protected:
    
    double sampleRate;
    int windowSize;
    int overlapFactor;
    int amountOfBins;
    double freqPerBin;
    double expectedPhase;
    int amountOfChannels;
    int amountOfTones;
    
    double fundamentalFreq;
    
    fftw_complex* freqCoefficients;
        
    double* lastPhase;
    double* sumPhase;
    double* analyticMagn;
    double* analyticFreq;
    double* syntheticMagn;
    double* syntheticFreq;

public:
    
    Keyboard(Processor*, int);
    ~Keyboard();
    int apply();
    int applyToChannel(unsigned int);
    
};

#endif
