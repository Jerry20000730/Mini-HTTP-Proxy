#include "proxy.hpp"

int main(int argc, char ** args) {
    int port = 12345;
    Proxy * proxy = new Proxy(port);
    proxy->run();

    return EXIT_SUCCESS;
}