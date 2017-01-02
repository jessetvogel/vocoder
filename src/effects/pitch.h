#ifndef PITCH_H
#define PITCH_H

#include <fftw3.h>
#include "effect.h"
#include "../processor.h"

class Pitch : public Effect {

protected:
    
    double sampleRate;
    int windowSize;
    int overlapFactor;
    int amountOfBins;
    double freqPerBin;
    double expectedPhase;
    int amountOfChannels;
    
    fftw_complex* freqCoefficients;
    
    double pitchShift;
    
    double* lastPhase;
    double* sumPhase;
    double* analyticMagn;
    double* analyticFreq;
    double* syntheticMagn;
    double* syntheticFreq;
    
public:
    
    Pitch(Processor*);
    ~Pitch();
    int apply();
    int applyToChannel(unsigned int);
    
};

#endif
