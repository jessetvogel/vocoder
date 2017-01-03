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

#include <iostream>

int zz = 0;

double util::fundamentalFrequency(double* analyticMagn, double* analyticFreq, int amountOfBins, double freqPerBin) {
    
    zz ++;
    
//    if(zz % 20 == 0) {
//        std::cout << std::endl;
//        for(int x = 0;x < 46;x ++) {
//            std::cout << analyticMagn[x] << '\t' << analyticFreq[x] << std::endl;
//        }
//        std::cout << std::endl;
//    }
//    
    
    // Uses Two-Way Mismatch (TWM) as described in: https://pdfs.semanticscholar.org/c94b/56f21f32b3b7a9575ced317e3de9b2ad9081.pdf
    
    // Declare variables
    double maxFreq = 2000;
    int maxPeaks = 8;
    double freqThreshold = freqPerBin / 2;
    double sumF, sumA2, peakFrequency, Amax = 0.0;
    unsigned int i, p, maxI = maxFreq / freqPerBin;
    
    // Step 1: Find peaks (simply check if it is a local maximum)
    double peakA[maxPeaks];
    double peakf[maxPeaks];
    
    p = 0;
    for(i = 1;i < maxI && p < maxPeaks;i ++) {
        // Look for local maxima
        if(analyticMagn[i] > analyticMagn[i - 1] && analyticMagn[i] > analyticMagn[i + 1]) {
            peakFrequency = analyticFreq[i];
            sumF = 0.0;
            sumA2 = 0.0;
            while(i > 0 && std::abs(analyticFreq[i - 1] - peakFrequency) < freqThreshold) {
                i --;
            }
            
            while(i < maxI && std::abs(analyticFreq[i] - peakFrequency) < freqThreshold) {
                sumF += analyticMagn[i] * analyticMagn[i] * analyticFreq[i];
                sumA2 += analyticMagn[i] * analyticMagn[i];
                i ++;
            }

            peakA[p] = std::sqrt(sumA2);
            peakf[p] = sumF / sumA2;
            if(peakA[p] > Amax) Amax = peakA[p];
            
            p ++;
        }
    }
    
    // Step 2: Choose fFund's
    unsigned int K = p;
    
    if(K == 0) return 0.0;
    
    double fFundMin = 50;
    double fFundMax = 500;
    double minError = 999999.999; // TODO fix this
    double fFund, fFundBest, ErrMtoP, ErrPtoM, ErrTotal;
    unsigned int N, n, k;
    double deltaFn, deltaFk, d;
    
    // Step 3+4+5+6: Compute for each fFund the error, and minimize it
    for(fFund = fFundMin;fFund < fFundMax;fFund *= 1.05946) {
        // Compute ErrPtoM
        ErrPtoM = 0.0;
        N = std::ceil(peakf[K - 1] / fFund);
        k = 0;
        for(n = 1;n <= N;n ++) {
            // Minimize deltaFn
            deltaFn = std::abs(fFund * n - peakf[k]);
            while(k + 1 < K && (d = std::abs(fFund * n - peakf[k + 1])) < deltaFn) {
                deltaFn = d;
                k ++;
            }
            double tmp = deltaFn / std::sqrt(fFund * n);
            ErrPtoM += tmp + (peakA[k] / Amax) * (1.4 * tmp - 0.5);
        }
        
        // Compute ErrMtoP
        ErrMtoP = 0.0;
        n = 0;
        for(k = 0;k < K;k ++) {
            // Minimize deltaFk
            n = (unsigned int) round(peakf[k] / fFund);
            deltaFk = std::abs(fFund * n - peakf[k]);
            
            double tmp = deltaFk / std::sqrt(peakf[k]);
            ErrMtoP += tmp + (peakA[k] / Amax) * (1.4 * tmp - 0.5);
        }
        
        // Compute ErrTotal
        ErrTotal = ErrPtoM / N + 0.33 * ErrMtoP / K;
        
        if(ErrTotal < minError) {
            minError = ErrTotal;
            fFundBest = fFund;
        }
    }
    
    // Return the frequency corresponding to the peak closest to fFundBest
    k = 0;
    deltaFn = std::abs(fFundBest - peakf[0]);
    while(k + 1 < K && (d = std::abs(fFund * n - peakf[k + 1])) < deltaFn) {
        deltaFn = d;
        k ++;
    }
    if(deltaFn < freqThreshold)
        return peakf[k];
    else
        return fFundBest;
}

