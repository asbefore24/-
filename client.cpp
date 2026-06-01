#include "client.h"

int main() {
    Client test;

    if (!test.connectServer(8080)) {
        return 0;
    }

    test.sendData();

    auto data = test.receiveData();
    test.closeClient();
    std::cout << data << std::endl;
    return 0;
}