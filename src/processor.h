#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <fftw3.h>
#include <vector>
#include <mutex>
#include "portaudio.h"
#include "definitions.h"
#include "effects/effect.h"

class Effect;
class Processor {

    PaStreamParameters* inputParameters;
    PaStreamParameters* outputParameters;
    
    PaSampleFormat sampleType;
    
    fftw_plan fft;
    fftw_plan ifft;
    
    std::mutex mutexThread;
    
    void error(const char*);
    
    static int callback(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
    
public:

    double sampleRate;
    int windowSize;
    int overlapFactor;
    int amountOfChannels;
    
    double* inputRaw;
    double* workspace;
    double* outputAccumulator;
    fftw_complex* frequencyCoefficients;
    
    std::vector<Effect*> effects;
    std::mutex mutexEffects;

    Processor(PaSampleFormat, int, int, int);
    int start();
    int stop();
    int run();
    ~Processor();
    
    int setInputDevice(int);
    int setOutputDevice(int);
    void getDeviceInfo();
    
    int inputDeviceId;
    int outputDeviceId;
    
    int running;
};

#endif
