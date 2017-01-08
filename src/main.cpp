#include <iostream>
#include "portaudio.h"

#include "processor.h"
#include "effects/pitch.h"
#include "effects/gain.h"
#include "effects/noisefilter.h"
#include "effects/equalizer.h"
#include "effects/autotune.h"
#include "effects/lowpass.h"
#include "effects/keyboard.h"

#include "server/server.h"
#include "server/commands.h"

#define MAX_LENGTH 1024

Processor* c;

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
                if(!Commands::execute(c, input))
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
    // Initialize Portaudio
    PaError err = Pa_Initialize();
    if(err != paNoError) {
        Pa_Terminate();
        std::cerr << "Failed to initialize portaudio." << std::endl;
        std::cerr << "Error number: " << err << std::endl;
        std::cerr << "Error message: " << Pa_GetErrorText(err) << std::endl;
        return 0;
    }
    
    // Setup processor
    c = new Processor(44100, 2048, 4, 2);
    
    // Setup server
    Server server(7000);
    server.handle = &callback;
    server.max_queue = 10;
    
    // Start server and processor
    server.start();
    c->start();

    getchar();
    
    server.stop();
    c->stop();
    
    // Clean up
    delete c;
    
    // Terminate Portaudio
    Pa_Terminate();
    
    return 0;
}
