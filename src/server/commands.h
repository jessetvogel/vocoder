#ifndef COMMANDS_H
#define COMMANDS_H

#include <regex>
#include "../processor.h"

class Commands {

    static std::regex regexUpdate;
    static std::regex regexRename;
    static std::regex regexAdd;
    static std::regex regexRemove;
    
public:
    
    static int execute(Processor*, char*);

};

#endif
