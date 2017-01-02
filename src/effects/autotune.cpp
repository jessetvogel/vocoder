#include "autotune.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <iostream>

int AutoTune::apply() {
    unsigned int k, maxBin;
    double real, imag, magn, phase, tmp, maxMagn = 0.0, maxFreq = 1.0;
    
    // Look for the instantaneous frequency with the highest magnitude (based on channel 1)
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude of each bin
        real = freqCoefficients[k][0];
        imag = freqCoefficients[k][1];
        magn = sqrt(real*real + imag*imag);
        
        // If it is the greatest, compute its true frequency
        if(magn > maxMagn && k != 0) {
            // Compute phase and phase difference
            phase = atan2(imag, real);
            tmp = phase - lastPhase[k];
            
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
            
            // Set new maximum magnitude and the belonging frequency
            maxMagn = magn;
            maxFreq = tmp;
        }
    }
    
    // Compute the pitch shift
    double semitones = round(log2(maxFreq / baseTone) * 12.0);
    options[0] = pow(2.0, semitones / 12.0) * baseTone / maxFreq;
    
    return Pitch::apply();
}
