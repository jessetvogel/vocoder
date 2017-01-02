#ifndef AUTOTUNE_H
#define AUTOTUNE_H

#include <fftw3.h>
#include "pitch.h"
#include "../processor.h"

class AutoTune : public Pitch {
    
    double baseTone;
    
public:

    AutoTune(Processor* processor) : Pitch(processor) {
        baseTone = 440.0;
    };
    int apply();
    
};

#endif
