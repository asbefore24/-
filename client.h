#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Client {
private:
    int sock_fd = -1;
    bool connected = false;
public:  
    Client() = default;

    bool connectServer(int port) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            std::cout << "Client create socket failed." << std::endl;
            return false;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        std::string server_ip = "127.0.0.1";
        if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cout << "Invalid address" << std::endl;
            return false;
        }
        if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cout << "Connection failed" << std::endl;
            return false;
        }

        connected = true;
        std::cout << "Connected success." << std::endl;
        return true;
    }

    bool sendData() {
        if (!connected) {
            return false;
        }

        int bytes_sent = send(sock_fd, "test", 4, 0);

        if (bytes_sent < 0) {
            std::cout << "Send failed" << std::endl;
            return false;
        }

        std::cout << "Sent success" << std::endl;
        return true;
    }

    std::string receiveData() {
        if (!connected) {
            return "";
        }

        char buffer[4096] = {0};
        int bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0) {
            std::cout << "Receive failed" << std::endl;
            return "";
        }

        if (bytes_read == 0) {
            std::cout << "Server disconnected" << std::endl;
            connected = false;
            return "";
        }

        return std::string(buffer, bytes_read);
    }

    void closeClient() {
        if (sock_fd >= 0) {
            close(sock_fd);
            connected = false;
            std::cout << "Connection closed" << std::endl;
        }
    }

    ~Client() {
        closeClient();
    }
};