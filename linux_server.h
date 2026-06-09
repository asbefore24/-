#include <string>
#include <unistd.h>
#include <functional>
#include <thread>
#include <vector>

#include "include/httpCode.h"
#include "include/user.h"

using namespace std;

class Server {
private:
    struct addrinfo *info = nullptr;
    int server_fd = 0;
    vector<thread> threadQueue;
    std::vector<UserMessage> messages;

private:
    void handleData(int client_fd);

    bool parseHttp(int client_fd, HttpRequest& request);

    void handleRequest(const HttpRequest& request, actionType action, string url, 
        std::function<void(const HttpRequest&)> callBack);

public:
    explicit Server(int port);
    
    ~Server();

    void run();
};