#include <iostream>
#include <string>
#include <cstring>
#ifndef _WIN32
#include <sys/socket.h>
#endif

class Server {
private:

public:
    Server() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            throw std::runtime_error("Socket creation failed");
        }
    };
    
};