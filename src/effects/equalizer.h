#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Equalizer : public Effect {
    
    double sampleRate;
    double freqPerBin;
    int amountOfBins;
    int amountOfChannels;
    
    double frequencyMin;
    double frequencyMax;
    int N;
    double logBase;
    
    fftw_complex* freqCoefficients;
    
public:
    
    Equalizer(Processor*, double, double, int);
    ~Equalizer();
    int apply();
    
};

#endif
