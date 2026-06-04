#include "linux_server.h"

#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <cstring>
#include <thread>
#include <queue>

#include "include/commonFunc.h"
#include "include/httpCode.h"

std::vector<std::thread> threadQueue;

Server::Server(int port) {
    if (getaddrinfo("127.0.0.1", "8080", nullptr, &info) != 0) {
        std::cout << "getaddrinfo failed" << std::endl;
        throw;
    }

    server_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (server_fd < 0) {
        std::cout << "create socket failed: " << std::strerror(errno) << std::endl;
        throw;
    }
    // 重用端口，防止每次服务器退出后需要等待
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, info->ai_addr, info->ai_addrlen) < 0) {
        std::cout << "Bind failed: " << std::strerror(errno) << std::endl;
        throw;
    }

    if (listen(server_fd, 20) < 0) {
        std::cout << "listen failed: " << std::strerror(errno) << std::endl;
        throw;
    }

    std::cout << "Server listening on port " << port << std::endl;
};

Server::~Server() {
    if (info) {
        freeaddrinfo(info);
    }
    if (server_fd >= 0) {
        close(server_fd);
    }
}

void Server::run() {
    while (true)
    {
        struct sockaddr_storage client_addr;
        socklen_t size = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &size);
        if (client_fd < 0)
        {
            std::cout << "accept failed: " << std::strerror(errno) << std::endl;
            continue;
        }
        cout << "Client connected: "<< client_fd << endl;
        threadQueue.emplace_back([this, client_fd]() {
            handleData(client_fd);
        });
    }
    for (auto& it : threadQueue) {
        it.join();
    }
}

void Server::handleData(int client_fd) {
    while (true) {
        cout << "handle data: " << client_fd << endl;
        auto res = parseHttp(client_fd);
        if (!res) {
            break;
        }
        HttpResponse response;
        auto fileString = CommonFunc::readFile("../resource/index.html");
        response.beginHead("HTTP/1.1", to_string(200), "OK");
        response.writeHead("Content-Type", "text/html");
        response.writeHead("connection", "keep-alive");
        response.writeHead("Content-length", to_string(fileString.length()));
        response.endHead();
        response.buffer += fileString;
        send(client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        cout << "handle data complete: " << client_fd << endl;
    }
    close(client_fd);
    cout << "Client close: " << client_fd << endl;
    return;
}

bool Server::parseHttp(int client_fd) {
    char buffer[1024] = {0};
    HttpRequest request;
    do {
        int n = read(client_fd, buffer, sizeof(buffer));
        if (n <= 0) {
            cout << "recv end" << client_fd << endl;
            return false;
        }
        request.push_chunk(string(buffer, n));
        memset(buffer, 0, sizeof(buffer));
    } while (!request.body_recv_end);
    return true;
}

int main() {
    try {
        Server it(8080);
        it.run();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}