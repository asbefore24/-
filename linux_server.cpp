#include "linux_server.h"

#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <cstring>

#include "include/commonFunc.h"

using json = nlohmann::json;

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
        HttpRequest request;
        request.client_fd = client_fd;
        auto res = parseHttp(client_fd, request);
        if (!res) {
            break;
        }
        // index页面
        handleRequest(request, actionType::GET, "/", [](const HttpRequest& request) {
            HttpResponse response;
            auto fileString = CommonFunc::readFile("../resource/index.html");
            response.beginHead("HTTP/1.1", to_string(200), "OK");
            response.writeHead("Content-Type", "text/html");
            response.writeHead("Connection", "keep-alive");
            response.writeHead("Content-Length", to_string(fileString.length()));
            response.endHead();
            response.buffer += fileString;
            send(request.client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        });
        // chatroom页面
        handleRequest(request, actionType::GET, "/chatroom", [](const HttpRequest& request) {
            HttpResponse response;
            auto fileString = CommonFunc::readFile("../resource/chatroom.html");
            response.beginHead("HTTP/1.1", to_string(200), "OK");
            response.writeHead("Content-Type", "text/html");
            response.writeHead("Content-Length", to_string(fileString.length()));
            response.endHead();
            response.buffer += fileString;
            send(request.client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        });
        // 登录请求
        handleRequest(request, actionType::POST, "/join", [](const HttpRequest& request) {
            HttpResponse response;
            User user = json::parse(request.body);
            response.beginHead("HTTP/1.1", to_string(200), "OK");
            response.writeHead("Content-Type", "application/json");
            json obj;
            // 校验账号
            if (userMap.count(user.username) && userMap[user.username] == user.password) {
                obj["success"] = ReturnCode::SUCCESS;
            } else {
                obj["success"] = ReturnCode::USER_ERROR;
            }
            string str = obj.dump();
            response.writeHead("Content-Length", to_string(obj.dump().length()));
            response.endHead();
            response.buffer += obj.dump();
            cout << response.buffer << endl;
            send(request.client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        });

        // 发送消息请求
        handleRequest(request, actionType::POST, "/sendMessage", [this](const HttpRequest& request) {
            HttpResponse response;
            UserMessage data = json::parse(request.body);
            response.beginHead("HTTP/1.1", to_string(200), "OK");
            response.writeHead("Content-Type", "application/json");
            response.endHead();
            messages.push_back(data);
            send(request.client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        });
        // 获取消息请求
        handleRequest(request, actionType::POST, "/recvMessage", [this](const HttpRequest& request) {
            HttpResponse response;
            response.beginHead("HTTP/1.1", to_string(200), "OK");
            response.writeHead("Content-Type", "application/json");
            json obj = messages;
            response.writeHead("Content-Length", to_string(obj.dump().length()));
            response.endHead();
            response.buffer += obj.dump(); 
            send(request.client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        });

        handleRequest(request, actionType::UNKNOWN, "", [](const HttpRequest& request) {
            HttpResponse response;
            response.beginHead("HTTP/1.1", to_string(200), "OK");
            response.endHead();
            send(request.client_fd, response.buffer.c_str(), response.buffer.length(), 0);
        });
    }
    close(client_fd);
    cout << "Client close: " << client_fd << endl;
    return; 
}

void Server::handleRequest(const HttpRequest& request, actionType action, string url, 
    std::function<void(const HttpRequest&)> callBack) {
    cout << "request: " << request.action << " " << request.url << endl;
    if ((request.action == action && request.url == url) || (action == actionType::UNKNOWN && url.empty())) {
        callBack(request);
    }
}

bool Server::parseHttp(int client_fd, HttpRequest& request) {
    char buffer[1024] = {0};
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