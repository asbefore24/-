#include <string>
#include <unistd.h>

using namespace std;

class Server {
private:
    struct addrinfo *info = nullptr;
    int server_fd = 0;

private:
    void handleData(int client_fd);

    bool parseHttp(int client_fd);

public:
    explicit Server(int port);
    
    ~Server();

    void run();
};