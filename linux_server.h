#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <filesystem>

#include "commonFunc.h"

using namespace std;

class Server {
private:
    int server_fd = 0;

public:
    explicit Server(int port) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        // 重用端口，防止每次退出后需要等待
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cout << "Bind failed: " << std::strerror(errno) << std::endl;
        }

        if (listen(server_fd, 20) < 0) {
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
            cout << "Waiting for connection..." << endl;
            struct sockaddr_storage their_addr;
            socklen_t size = sizeof(their_addr);
            int client_fd = accept(server_fd, (struct sockaddr*)&their_addr, &size);
            if (client_fd < 0) {
                cerr << "Accept failed" << endl;
                continue;
                }
            cout << "Client connected" << endl;
            // handle
            handleData(client_fd);
            close(client_fd);
            cout << "handle complete, Client disconnected" << endl;
        }
    }

    void handleData(int client_fd) {
        char buffer[1024] = {0};

        int bytes_recv = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recv <= 0) {
            return;
        }
        buffer[bytes_recv] = '\0';
        // parse
        auto res = parseHttp(buffer, bytes_recv);
        string test = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\n\r\n";
        auto fileString = CommonFunc::readFile("../resource/index.html");
        test += fileString;
        std::cout << "test: " << test << std::endl;
        send(client_fd, test.c_str(), test.length(), 0);
        return;
    }

    string parseHttp(char* buffer, size_t len) {
        return "";
    }
};