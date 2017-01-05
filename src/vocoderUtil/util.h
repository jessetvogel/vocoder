#ifndef UTIL_H
#define UTIL_H

#include <fftw3.h>

namespace util {

    int analysis (fftw_complex*, double*, int, int, double, double*, double*);
    int synthesis(fftw_complex*, double*, int, int, double, double*, double*);
    int synthesisAdd(fftw_complex*, double*, int, int, double, double*, double*);
    
    double fundamentalFrequency(double*, double*, int, double);
    double TWMError(double, double*, double*, int, double);
}

#endif
