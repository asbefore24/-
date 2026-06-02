#include "test_client.h"

int main() {
    Client test;

    if (!test.connectServer(8080)) {
        return 0;
    }
    std::cout << "start send data." << std::endl;
    test.sendData();

    auto data = test.receiveData();
    test.closeClient();
    std::cout << "client receive data:" << data << std::endl;
    return 0;
}