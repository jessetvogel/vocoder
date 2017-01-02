#include "commands.h"
#include <cstdlib>
#include "../effects/effect.h"
#include "../effects/gain.h"
#include "../effects/pitch.h"
#include "../effects/equalizer.h"
#include "../effects/echo.h"

#include <iostream>

std::regex Commands::regexUpdate("^UPDATE (\\w+) (\\w+) ((?:[-+]?[0-9]+\\.?[0-9]*)(?:,[-+]?[0-9]+\\.?[0-9]*)*)$");
std::regex Commands::regexRename("^RENAME (\\w+) (\\w{1,16})$");
std::regex Commands::regexAdd("^ADD (\\w+) (\\w{1,16})(?: ((?:[-+]?[0-9]+\\.?[0-9]*)(?:,[-+]?[0-9]+\\.?[0-9]*)*))?$");
std::regex Commands::regexRemove("^REMOVE (\\w+)$");

// TODO!! CHECK ALL INPUT FOR FORMATING!
int Commands::execute(Processor* processor, char* command) {
    std::cmatch cm;
    
    // Check whether they match the regex's
    
    // UPDATE effect label values
    if(std::regex_search(command, cm, regexUpdate)) {
        // Find the corresponding effect
        for(auto itEffect = processor->effects.begin(); itEffect != processor->effects.end();itEffect ++) {
            if(cm[1].compare((*itEffect)->name) == 0) {
                // Find the corresponding label
                Effect* effect = (*itEffect);
                for(auto itLabel = effect->optionLabels.begin(); itLabel != effect->optionLabels.end();itLabel ++) {
                    if(cm[2].compare((*itLabel).label) == 0) {
                        // Set the corresponding options TODO: do this in some nicer way
                        std::string str = cm[3].str();
                        char* cstr = new char[str.length() + 1];
                        strcpy(cstr, str.c_str());
                        int option = (*itLabel).option;
                        char* ptr = cstr;
                        char* end = cstr + str.length();
                        while(ptr < end && option < effect->amountOfOptions) {
                            effect->options[option] = atof(ptr);
                            while(ptr < end && *ptr != ',') ptr ++; ptr ++;
                            option ++;
                        }
                        delete[] cstr;
                        return 1;
                    }
                }
            }
        }
    }
    
    // RENAME oldName newName
    if(std::regex_search(command, cm, regexRename)) {
        // Find the corresponding effect
        for(auto it = processor->effects.begin(); it != processor->effects.end();it ++) {
            if(cm[1].compare((*it)->name) == 0) {
                // Rename the effect
                strcpy((*it)->name, cm[2].str().c_str());
            }
        }
    }
    
    // ADD type name parameters
    if(std::regex_search(command, cm, regexAdd)) {
        Effect* effect = nullptr;
        
        // Parse parameters if given
        std::string str = cm[3].str();
        int amountofParameters = std::count(str.begin(), str.end(), ',') + 1;
        double parameters[amountofParameters];
        if(str.length() > 0) { // Check if there are any parameters given
            const char* ptr = str.c_str();
            for(unsigned int i = 0;i < amountofParameters;i ++) {
                parameters[i] = atof(ptr);
                while(*ptr != ',') ptr ++; ptr ++;
            }
        }
        
        // List of supported effects
        if(cm[1].compare("gain") == 0)
            effect = new Gain(processor);

        else if(cm[1].compare("pitch") == 0)
            effect = new Pitch(processor);

        else if(cm[1].compare("echo") == 0)
            effect = new Echo(processor);

        else if(cm[1].compare("equalizer") == 0)
            effect = new Equalizer(processor, parameters[0], parameters[1], (int) (parameters[2]));
        
        if(effect != nullptr) {
            strcpy(effect->name, cm[2].str().c_str());
            processor->effects.push_back(effect);
            return 1;
        }
    }
    
    // REMOVE effectName
    if(std::regex_search(command, cm, regexRemove)) {
        // Find the corresponding effect
        for(auto it = processor->effects.begin();it != processor->effects.end();it ++) {
            if(cm[1].compare((*it)->name) == 0) {
                // Remove it from the list
                processor->effects.erase(it);
                return 1;
            }
        }
    }
    
    return 0;
}
