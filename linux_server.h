#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

class Server {
private:
    int server_fd = 0;
    struct sockaddr_in address;

public:
    explicit Server(int port) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        if (listen(server_fd, 5) < 0) {
            throw std::runtime_error("Listen failed");
        }

        std::cout << "Server listening on port " << port << std::endl;
    };
    
    ~Server() {
        if (server_fd >= 0) {
            close(server_fd);
        }
    }

    void run() {
        while (true) {
            std::cout << "Waiting for connection..." << std::endl;
            socklen_t addrlen = sizeof(address);
            int client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
            if (client_fd < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            std::cout << "Client connected";
            // handle
            handleData(client_fd);

            std::cout << "handle complete";
            close(client_fd);
            std::cout << "Client disconnected" << std::endl;
        }
    }

    void handleData(int client_fd) {
        char buffer[1024] = {0};

        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            return;
        }
        buffer[bytes_read] = '\0';
        std::cout << "[Server] Received: " << buffer << std::endl;
        std::string response = "Server echo: " + std::string(buffer);
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }
};