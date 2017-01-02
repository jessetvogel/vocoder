#include "server.h"

Server::Server(int port) {
    // Set values
    this->port = port;
}

int Server::start() {
    // First open socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        fail("opening socket");
        return 0;
    }
    
    // Initialize socket structures
    memset(&server_socket_info, 0, sizeof(server_socket_info));
    server_socket_info.sin_family = AF_INET;
    server_socket_info.sin_addr.s_addr = INADDR_ANY;
    server_socket_info.sin_port = htons(port);
    
    // Bind server_socket_information to the opened socket
    if(bind(server_socket, (struct sockaddr*) &server_socket_info, sizeof(sockaddr)) < 0) {
        fail("binding");
        return 0;
    }
    
    // Create a new thread to run the server in
    std::thread server_loop(&Server::run, this);
    server_loop.detach();
    
    return 1;
}

void Server::run() {
    // Start listening
    listen(server_socket, max_queue);
    
    socklen_t addr_len = sizeof(dest_socket_info);
    
    running = 1;
    while(running) {
        // Accept incoming clients
        int socket_client = accept(server_socket, (struct sockaddr*) &dest_socket_info, &addr_len);
        
        if(socket_client < 0) {
            if(!running) return;
            fail("accepting client");
            continue;
        }
        
        std::thread client_handle(&Server::handle_client, this, socket_client);
        client_handle.detach();
    }
}

void Server::stop() {
    // Stop running, and close the server socket
    running = 0;
    close(server_socket);
}

void Server::fail(const char *message) {
    std::cout << "[SERVER ERROR] " << message << std::endl;
}

Server::~Server() {
    // Close the server socket (if still running)
    if(running && close(server_socket) < 0) {
        fail("closing socket");
    }
    running = 0;
}
