#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <thread>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

class Server {
    
    int port;
    int server_socket;
    
    struct sockaddr_in server_socket_info;
    struct sockaddr_in dest_socket_info;
    
    bool running;
    void run();
    
    int handle_client(int socket_client) { return handle(socket_client); }
    
    void fail(const char*);
    
public:
    
    Server(int);
    ~Server();
    
    int start();
    void stop();
    
    int (*handle)(int);
    
    int max_queue;

};

#endif
