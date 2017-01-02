#ifndef EFFECT_H
#define EFFECT_H

#include <vector>

#define EFFECT_NAME_LENGTH 16
#define OPTION_NAME_LENGTH 16

struct OptionLabel {
    int option;
    char label[OPTION_NAME_LENGTH];
    OptionLabel(const char* label, int option) {
        this->option = option;
        strcpy(this->label, label);
    }
};

class Effect {
    
public:
    
    char name[EFFECT_NAME_LENGTH];
    int amountOfOptions;
    double* options;
    std::vector<OptionLabel> optionLabels;
    
    virtual int apply() {
        return 0;
    };
    
    virtual ~Effect() {};
};

#endif
