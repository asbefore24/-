#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

class Client {
private:
    int sock_fd = -1;
    bool connected = false;
public:
    Client() = default;

    bool connect(int port) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
};