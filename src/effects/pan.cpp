#include "pan.h"
#include <cmath>

Pan::Pan(Processor* processor) {
    // Store and compute useful information
    freqCoefficients = processor->frequencyCoefficients;
    amountOfBins = processor->windowSize/2 + 1;
    
    // Set (default) name and options
    strcpy(name, "Pan");
    amountOfOptions = 1;
    options = new double[amountOfOptions];
    
    optionLabels.push_back(OptionLabel("panning", 0));
    options[0] = 0.0;
}

int Pan::apply() {
    // Keep the panning parameter in [-1.0, 1.0]
    double panning = options[0];
    if(panning < -1.0) panning = -1.0;
    if(panning >  1.0) panning =  1.0;
    
    // Declare variables
    unsigned int k;
    
    // For every bin, multiply its value by gain
    for(k = 0;k < amountOfBins;k ++) {
        // Compute magnitude and phase of each bin
        freqCoefficients[k][0] *= cos(panning * M_PI / 2);
        freqCoefficients[k][1] *= cos(panning * M_PI / 2);
        
        // TODO: implement right channel
    }
    
    return 1;
}
