#include "linux_server.h"

int main() {
    try {
        Server it(8080);
        it.run();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}