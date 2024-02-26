#include "proxy.hpp"

int main(int argc, char ** args) {
    int port = 9999;
    Proxy * proxy = new Proxy(port);
    proxy->run();

    delete proxy;
    delete Proxy::cache;
    Proxy::cache = nullptr;
    delete Proxy::log;
    Proxy::log = nullptr; // 防止野指针
    return EXIT_SUCCESS;
}