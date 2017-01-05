#include <iostream>

#include "processor.h"
#include "effects/pitch.h"
#include "effects/gain.h"
#include "effects/noisefilter.h"
#include "effects/equalizer.h"
#include "effects/autotune.h"
#include "effects/keyboard.h"

#include "server/server.h"
#include "server/commands.h"

#define MAX_LENGTH 1024

Processor c(44100, 1024, 8, 2);

int callback(int socket_client)
{
    std::cout << "Interface connected." << std::endl;
    
    char input[MAX_LENGTH];
    
    int running = 1;
    while(running) {
        memset(input, 0, MAX_LENGTH);
        char* ptr = input;
        while(ptr - input < MAX_LENGTH && read(socket_client, ptr, 1) > 0) { // Read byte by byte, TODO: add buffering or something like that
            if(*ptr == '\n') {
                *ptr = '\0'; // For easier regex stuff...
                if(!Commands::execute(&c, input))
                    std::cout << "Command '" << input << "' not recognised!" << std::endl;
                break;
            }
            ptr ++;
        }
    }
    
    close(socket_client);
    
    std::cout << "Interface disconnected." << std::endl;
    return 1;
}

int main() {
    Server server(7000);
    server.handle = &callback;
    server.max_queue = 10;

    server.start();
    c.start();
    
    getchar();
    
    server.stop();
    c.stop();
    
    return 0;
}
