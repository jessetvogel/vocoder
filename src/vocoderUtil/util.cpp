#include "util.h"
#include <cmath>

int util::analysis(fftw_complex* freqCoefficients, double* lastPhase, int amountOfBins, int overlapFactor, double freqPerBin, double* analyticMagn, double* analyticFreq) {
    // Declare variables
    double magn, phase, tmp, real, imag, expectedPhase = 2.0 * M_PI / overlapFactor;
    unsigned int k;
    
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude and phase of each bin
        real = freqCoefficients[k][0];
        imag = freqCoefficients[k][1];
        
        magn = sqrt(real*real + imag*imag);
        phase = atan2(imag, real);
        
        // Compute phase difference, and update lastPhase for the next buffer
        tmp = phase - lastPhase[k];
        lastPhase[k] = phase;
        
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
    
    return 1;
}

int util::synthesis(fftw_complex* freqCoefficients, double* sumPhase, int amountOfBins, int overlapFactor, double freqPerBin, double* syntheticMagn, double* syntheticFreq) {
    // Declare variables
    double magn, phase, tmp, expectedPhase = 2.0 * M_PI / overlapFactor;
    unsigned int k, index;
    
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
        sumPhase[k] += tmp;
        phase = sumPhase[k];
        
        // Compute real and imaginary part
        freqCoefficients[k][0] = magn * cos(phase);
        freqCoefficients[k][1] = magn * sin(phase);
    }
    
    return 1;
}

int util::synthesisAdd(fftw_complex* freqCoefficients, double* sumPhase, int amountOfBins, int overlapFactor, double freqPerBin, double* syntheticMagn, double* syntheticFreq) {
    // Declare variables
    double magn, phase, tmp, expectedPhase = 2.0 * M_PI / overlapFactor;
    unsigned int k, index;
    
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
        sumPhase[k] += tmp;
        phase = sumPhase[k];
        
        // Compute real and imaginary part
        freqCoefficients[k][0] += magn * cos(phase);
        freqCoefficients[k][1] += magn * sin(phase);
    }
    
    return 1;
}

int zz = 0;

#include <iostream>

double util::fundamentalFrequency(double* analyticMagnNoFilter, double* analyticFreq, int amountOfBins, double freqPerBin) {
    // Uses Two-Way Mismatch (TWM) as described in: https://pdfs.semanticscholar.org/c94b/56f21f32b3b7a9575ced317e3de9b2ad9081.pdf
    
    // Declare variables
    const double freqRange = 2000.0;
    const int maxAmountOfPeaks = 12;
    const double freqThreshold = freqPerBin / 2;
    int maxBin = freqRange / freqPerBin; if(maxBin > amountOfBins) maxBin = amountOfBins;
    double sumFreq, sumMagn2, magn2, localFreq, maximumMagn = 0.0;
    int bin, l, p;
    
    // First apply low pass filter to make better prediction
    double analyticMagn[amountOfBins];
    for(bin = 0;bin < amountOfBins;bin ++) {
        analyticMagn[bin] = analyticMagnNoFilter[bin] / ((1.0 + analyticFreq[bin] / 500.0) * (1.0 + analyticFreq[bin] / 500.0));
    }
    
    // Step 1: Find peaks (simply check if it is a local maximum)
    double peakMagn[maxAmountOfPeaks];
    double peakFreq[maxAmountOfPeaks];
    
    p = 0;
    for(bin = 1;bin < maxBin && p < maxAmountOfPeaks;bin ++) {
        // Look for local maxima
        if(analyticMagn[bin] > analyticMagn[bin - 1] && analyticMagn[bin] > analyticMagn[bin + 1]) {
            localFreq = analyticFreq[bin];
            magn2 = analyticMagn[bin]*analyticMagn[bin];
            sumMagn2 = magn2;
            sumFreq = localFreq * magn2;
            
            // Search for bins below this one contributing to (roughly) the same frequency
            l = bin - 1;
            while(l >= 0 && std::abs(analyticFreq[l] - localFreq) < freqThreshold) {
                magn2 = analyticMagn[l] * analyticMagn[l];
                sumMagn2 += magn2;
                sumFreq += analyticFreq[l] * magn2;
                l --;
            }
            
            // Search for bins above this one contributing to (roughly) the same frequency
            while(bin + 1 < maxBin && std::abs(analyticFreq[bin + 1] - localFreq) < freqThreshold) {
                bin ++;
                magn2 = analyticMagn[bin]*analyticMagn[bin];
                sumMagn2 += magn2;
                sumFreq += analyticFreq[bin] * magn2;
            }

            // Update maximumMagn if the magnitude of this peak is larger than the previous one
            peakMagn[p] = std::sqrt(sumMagn2);
            peakFreq[p] = sumFreq / sumMagn2;
            if(peakMagn[p] > maximumMagn) maximumMagn = peakMagn[p];
            
            // Next peak
            p ++;
        }
    }
    
    // If no peaks were found, return that the fundamental frequency is zero
    if(p == 0) return 0.0;
    
    // Step 2: Choose fundamental frequencies
    int K = p;
    double fundamentalFreqMin = 50;
    double fundamentalFreqMax = 500;
    double minimumError = 999999.999; // TODO fix this
    double fundamentalFreq, fundamentalFreqBest;
    
    // Step 3+4+5+6: Compute for each fundamentalFreq the error, and minimize it (take steps of 1 semitone)
    for(fundamentalFreq = fundamentalFreqMin;fundamentalFreq < fundamentalFreqMax;fundamentalFreq *= 1.059463094359295) {
        double error = TWMError(fundamentalFreq, peakMagn, peakFreq, K, maximumMagn);
        if(error < minimumError) {
            minimumError = error;
            fundamentalFreqBest = fundamentalFreq;
        }
    }
    
    // Refine a bit more (8 steps per semitone)
//    fundamentalFreqMin = fundamentalFreqBest * 0.950714015038751;
//    fundamentalFreqMax = fundamentalFreqBest * 1.051841020729289;
//    for(fundamentalFreq = fundamentalFreqMin;fundamentalFreq < fundamentalFreqMax;fundamentalFreq *= 1.007246412223704) {
//        double error = TWMError(fundamentalFreq, peakMagn, peakFreq, K, maximumMagn);
//        if(error < minimumError) {
//            minimumError = error;
//            fundamentalFreqBest = fundamentalFreq;
//        }
//    }
    
    if(minimumError < 0.33)
        certainty = 1.0;
    else if(minimumError > 1.33)
        certainty = 0.0;
    else
        certainty = (1.33 - minimumError) / 1.66;
    
    return fundamentalFreqBest;
}

double util::TWMError(double fundamentalFreq, double* peakMagn, double* peakFreq, int K, double maximumMagn) {
    // Compute errorPToM
    double errorPToM = 0.0;
    int N = std::ceil(peakFreq[K - 1] / fundamentalFreq);
    int k = 0;
    double d;
    for(int n = 1;n <= N;n ++) {
        // Minimize deltaFn
        double deltaFn = std::abs(fundamentalFreq * n - peakFreq[k]);
        while(k + 1 < K && (d = std::abs(fundamentalFreq * n - peakFreq[k + 1])) < deltaFn) {
            deltaFn = d;
            k ++;
        }
        double tmp = deltaFn / std::sqrt(fundamentalFreq * n);
        errorPToM += tmp + (peakMagn[k] / maximumMagn) * (1.4 * tmp - 0.5);
    }
    
    // Compute errorMToP
    double errorMToP = 0.0;
    for(k = 0;k < K;k ++) {
        // Minimize deltaFk
        double deltaFk = std::abs(fundamentalFreq * round(peakFreq[k] / fundamentalFreq) - peakFreq[k]);
        
        double tmp = deltaFk / std::sqrt(peakFreq[k]);
        errorMToP += tmp + (peakMagn[k] / maximumMagn) * (1.4 * tmp - 0.5);
    }
    
    // Return the total error
    return errorPToM / N + 0.33 * errorMToP / K;
}

double util::certainty = 0.0;
