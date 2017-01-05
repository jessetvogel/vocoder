#include "processor.h"

#include <iostream>
#include <math.h>
#include <ctime>
#include <thread>

Processor::Processor(PaSampleFormat sampleRate, int windowSize, int overlapFactor, int amountOfChannels) {
    // Store the givens TODO: pass the options in another way, maybe something like a struct?
    this->sampleType = paFloat32;
    this->sampleRate = sampleRate;
    this->windowSize = windowSize;
    this->overlapFactor = overlapFactor;
    this->amountOfChannels = amountOfChannels;
    
    // Create input, output and frequency coefficient buffers
    inputRaw = new double[windowSize * amountOfChannels];
    workspace = new double[windowSize * amountOfChannels];
    outputAccumulator = new double[windowSize * amountOfChannels];
    frequencyCoefficients = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (windowSize/2 + 1) * amountOfChannels);
    
    memset(inputRaw, 0, sizeof(double) * windowSize * amountOfChannels);
    memset(workspace, 0, sizeof(double) * windowSize * amountOfChannels);
    memset(outputAccumulator, 0, sizeof(double) * windowSize * amountOfChannels);
    memset(frequencyCoefficients, 0, sizeof(fftw_complex) * (windowSize/2 + 1) * amountOfChannels);
    
    // Create FFT and IFFT plans
    fft = fftw_plan_dft_r2c_1d(windowSize, workspace, frequencyCoefficients, FFTW_ESTIMATE);  //Setup fftw plan for fft
    ifft = fftw_plan_dft_c2r_1d(windowSize, frequencyCoefficients, workspace, FFTW_ESTIMATE);   //Setup fftw plan for ifft
}

int Processor::start() {
    // Execute run in a different thread
    mutexThread.lock();
    std::thread thread(&Processor::run, this);
    thread.detach();
    return 1;
}

int Processor::stop() {
    // Unlock mutex, so that the thread will terminate
    mutexThread.unlock();
    return 1;
}

int Processor::run() {
    PaError err;
    
    err = Pa_Initialize();
    if(err != paNoError) goto error;
    
    // Set stream/input/output options
    PaStream *stream;
    
    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if(inputParameters.device == paNoDevice) {
        error("Error: No default input device.");
        goto error;
    }
    inputParameters.channelCount = 2;       /* stereo input */
    inputParameters.sampleFormat = sampleType;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if(outputParameters.device == paNoDevice) {
        error("Error: No default input device.");
        goto error;
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = sampleType;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    // Create a stream
    err = Pa_OpenStream(
                        &stream,
                        &inputParameters,
                        &outputParameters,
                        sampleRate,
                        windowSize / overlapFactor, // TODO: create a variable for this?
                        0, /* paClipOff, */  /* we won't output out of range samples so don't bother clipping them */
                        callback,
                        this );
    
    if(err != paNoError) goto error;
    
    err = Pa_StartStream(stream);
    if(err != paNoError) goto error;
    
    // Wait for mutex to unlock
    mutexThread.lock();
    mutexThread.unlock();
    
    err = Pa_CloseStream(stream);
    if(err != paNoError) goto error;
    
    Pa_Terminate();
    return 0;
    
error:
    // Whenever an error occured, terminate portaudio and show the error
    Pa_Terminate();
    std::cerr << "An error occured while using the portaudio stream" << std::endl;
    std::cerr << "Error number: " << err << std::endl;
    std::cerr << "Error message: " << Pa_GetErrorText(err) << std::endl;
    return -1;
}

int Processor::callback(const void* inputBuffer,
                         void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData) {
    // Declare some variables already
    const SAMPLE *in = (const SAMPLE*) inputBuffer;
    SAMPLE *out = (SAMPLE*) outputBuffer;
    unsigned int x, channel;
    double window;
    
    // Prevent unused variable warnings.
    (void) timeInfo;
    (void) statusFlags;
    
    // Get relevant processor object
    Processor* processor = (Processor*) userData;
    double* inputRaw = processor->inputRaw;
    double* workspace = processor->workspace;
    double* outputAccumulator = processor->outputAccumulator;
    int amountOfChannels = processor->amountOfChannels;
    std::vector<Effect*> effects = processor->effects;
    int windowSize = processor->windowSize;
    int overlapFactor = processor->overlapFactor;
    fftw_complex* frequencyCoefficients = processor->frequencyCoefficients;
    
    // If inputBuffer is NULL, just send silence to the output
    if(inputBuffer == NULL) {
        for(x = 0;x < framesPerBuffer;x ++) {
            for(channel = 0;channel < amountOfChannels;channel ++) {
                *out++ = 0.0;
            }
        }
        return paContinue;
    }
    
    // Shift inputRaw
    for(channel = 0;channel < amountOfChannels;channel ++) {
        memmove(inputRaw + channel * windowSize, inputRaw + channel * windowSize + framesPerBuffer, sizeof(double) * framesPerBuffer * (overlapFactor - 1));
    }
    
    // Copy buffer into inputRaw
    double* ptr = inputRaw + framesPerBuffer * (overlapFactor - 1);
    for(x = 0;x < framesPerBuffer;x ++) {
        for(channel = 0;channel < amountOfChannels;channel ++) {
            *(ptr + channel * windowSize) = (double) (*in++);
        }
        ptr ++;
    }
    
    // Window the raw input to workspace (using Hann window) TODO: store window, since it is the same every buffer. TODO: maybe custom window later?
    for(x = 0;x < windowSize;x ++) {
        window = 0.5 - 0.5 * cos(2.0 * M_PI * (double) x / (double) windowSize);
        for(channel = 0;channel < amountOfChannels;channel ++) {
            workspace[x + channel * windowSize] = inputRaw[x + channel * windowSize] * window;
        }
    }
    
    // FFT
    for(channel = 0;channel < amountOfChannels;channel ++)
        fftw_execute_dft_r2c(processor->fft, workspace + channel * windowSize, frequencyCoefficients + channel * (windowSize/2 + 1));
    
    // Apply effects
    processor->mutexEffects.lock();
    for(auto it = effects.begin(); it != effects.end();it ++)
        (*it)->apply();
    processor->mutexEffects.unlock();
    
    // IFFT
    for(channel = 0;channel < amountOfChannels;channel ++)
        fftw_execute_dft_c2r(processor->ifft, frequencyCoefficients + channel * (windowSize/2 + 1), workspace + channel * windowSize);
    
    // Shift accumulator
    for(channel = 0;channel < amountOfChannels;channel ++) {
        memmove(outputAccumulator + channel * windowSize, outputAccumulator + channel * windowSize + framesPerBuffer, sizeof(double) * framesPerBuffer * (overlapFactor - 1));
        memset(outputAccumulator + channel * windowSize + framesPerBuffer * (overlapFactor - 1), 0, sizeof(double) * framesPerBuffer);
    }
    
    // Windowing and add to output accumulator
    for(x = 0;x < windowSize;x ++) {
        window = 0.5 - 0.5 * cos(2.0 * M_PI * (double) x / (double) windowSize);
        for(channel = 0;channel < amountOfChannels;channel ++) {
            outputAccumulator[x + channel * windowSize] += 2.0 * window * workspace[x + channel * windowSize] / (windowSize / 2 * overlapFactor);
        }
    }
    
    // Send the accumulated output to the output
    for(x = 0;x < framesPerBuffer;x ++) {
        for(channel = 0;channel < amountOfChannels;channel ++) {
            *out++ = (SAMPLE) outputAccumulator[x + channel * windowSize];
        }
    }
    
    return paContinue;
}

void Processor::error(const char* message) {
    // Print message to stderr
    std::cerr << message << std::endl;

}

Processor::~Processor() {
    // Deallocate everything
    delete[] inputRaw;
    delete[] workspace;
    delete[] outputAccumulator;
    fftw_free(frequencyCoefficients);
    
    // Delete all effects
    for(auto it = effects.begin(); it != effects.end();it ++) {
        delete (*it);
    }
}
