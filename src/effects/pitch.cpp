#include "pitch.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

Pitch::Pitch(Processor* processor) {
    // Store and compute useful information
    sampleRate = processor->sampleRate;
    windowSize = processor->windowSize;
    overlapFactor = processor->overlapFactor;
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    amountOfChannels = processor->amountOfChannels;
    freqPerBin = sampleRate / processor->windowSize;
    expectedPhase = 2.0 * M_PI / overlapFactor;
    
    // Create buffers
    lastPhase = new double[amountOfBins * amountOfChannels];
    sumPhase = new double[amountOfBins * amountOfChannels];
    analyticMagn = new double[amountOfBins];
    analyticFreq = new double[amountOfBins];
    syntheticMagn = new double[amountOfBins];
    syntheticFreq = new double[amountOfBins];
    
    memset(lastPhase, 0, sizeof(double) * amountOfBins * amountOfChannels);
    memset(sumPhase, 0, sizeof(double) * amountOfBins * amountOfChannels);
    memset(analyticMagn, 0, sizeof(double) * amountOfBins);
    memset(analyticFreq, 0, sizeof(double) * amountOfBins);
    memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
    memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
    
    // Set (default) name and options
    strcpy(name, "Pitch");
    amountOfOptions = 1;
    options = new double[amountOfOptions];

    optionLabels.push_back(OptionLabel("shift", 0)); options[0] = 1.0;
}

int Pitch::apply() {
    // Fix pitchShift (in case of asynchronous setting of options)
    pitchShift = options[0];
    
    // Apply a pitch shift to all channels
    for(unsigned int channel = 0;channel < amountOfChannels;channel ++)
        applyToChannel(channel);
    
    return 1;
}

int Pitch::applyToChannel(unsigned int channel) {
    int offset  = channel * amountOfBins;
    fftw_complex* _freqCoefficients = freqCoefficients + offset;
    double* _lastPhase = lastPhase + offset;
    double* _sumPhase = sumPhase + offset;
    
    // Declare variables
    double magn, phase, tmp, real, imag;
    unsigned int k, index;
    
    // Step 1: Analysis
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude and phase of each bin
        real = _freqCoefficients[k][0];
        imag = _freqCoefficients[k][1];
        
        magn = sqrt(real*real + imag*imag);
        phase = atan2(imag, real);
        
        // Compute phase difference, and update lastPhase for the next buffer
        tmp = phase - _lastPhase[k];
        _lastPhase[k] = phase;
        
        // Subtract expected phase difference
        tmp -= (double) k * expectedPhase;
        
        // Map delta phase into +/- Pi interval (TODO: this is just MAGIC! you might want to look sometime at how this was actually done...)
        long qpd = tmp / M_PI;
        if (qpd >= 0) qpd += qpd & 1;
        else qpd -= qpd & 1;
        tmp -= M_PI * (double) qpd;
        
        // Get deviation from bin frequency from the +/- Pi interval
        tmp = overlapFactor * tmp / (2.0 * M_PI);
        
        // Compute the k-th instantaneous frequency
        tmp = freqPerBin * ((double) k + tmp);
        
        // Store magnitude and instantaneous frequency in array
        analyticMagn[k] = magn;
        analyticFreq[k] = tmp;
    }
    
    // Step 2: Processing
    memset(syntheticMagn, 0, sizeof(double) * amountOfBins);
    memset(syntheticFreq, 0, sizeof(double) * amountOfBins);
    for(k = 0; k < amountOfBins;k ++) {
        index = round(k * pitchShift);
        if (index < amountOfBins) {
            syntheticMagn[index] += analyticMagn[k];
            syntheticFreq[index] = analyticFreq[k] * pitchShift;
        }
    }
    
    // Step 3: Synthesis
    for(k = 0;k < amountOfBins;k ++) {
        // Get magnitude and instantaneous frequency from synthesis arrays
        magn = syntheticMagn[k];
        tmp = syntheticFreq[k];
        
        // Subtract bin mid frequency
        tmp -= (double) k * freqPerBin;
        
        // Get bin deviation from freq deviation
        tmp /= freqPerBin;
        
        // Take overlapFactor into account
        tmp = 2.0 * M_PI * tmp / overlapFactor;
        
        // Add the overlap phase advance back in
        tmp += (double) k * expectedPhase;
        
        // Accumulate delta phase to get bin phase
        _sumPhase[k] += tmp;
        phase = _sumPhase[k];
        
        // Compute real and imaginary part
        _freqCoefficients[k][0] = magn * cos(phase);
        _freqCoefficients[k][1] = magn * sin(phase);
    }
    
    return 1;
}

Pitch::~Pitch() {
    // Deallocate memory TODO: check in all other files if also done "delete[]" correctly
    delete[] lastPhase;
    delete[] sumPhase;
    delete[] analyticMagn;
    delete[] analyticFreq;
    delete[] syntheticMagn;
    delete[] syntheticFreq;
    delete[] options;
}
