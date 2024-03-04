#ifndef _PROXY_HPP_
#define _PROXY_HPP_

#include <netdb.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <csignal>

#include "cache.hpp"
#include "log.hpp"
#include "request.hpp"
#include "response.hpp"

struct _info {
    int id;
    int server_socket_fd;
    int server_connect_fd;
    std::string ip;

    _info() : id(-1), server_socket_fd(-1), server_connect_fd(-1) {}
};
typedef struct _info info;

class Proxy {
public:
    int port;
    std::unique_ptr<LRUCache> cache;
    std::unique_ptr<Log> log;

    // rule of three
    Proxy(int port);
    ~Proxy();

    // main function
    void * task(void * info);
    void run();

    // server start function
    int initServer(const char * port);
    void acceptConnection(int * socket_fd, int * connect_fd, std::string * ip);

    // client start function
    int initClient(const char * hostname, const char * port, std::string method);

    // request / response send/recv functions
    std::string recvClientRequest(int * connect_fd);
    void sendRequestToServer(int * socket_fd, std::string request);
    void sendResponseToClient(int * socket_fd, std::vector<char> & response);
    void sendResponseToClient(int * socket_fd, Response * response);
    void recvServerResponse(int * server_fd, int * client_fd, Response * response);
    std::vector<char> recvServerChunkedResponse(int * server_fd, int * client_fd);
    std::vector<char> recvServerLongResponse(int * server_fd);
    // process request
    void processConnect(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id);
    void processGet(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, std::string * url, int id);
    // static void processLongGet(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id);
    void processPost(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id);
    bool revalidateResponse(int * client_fd, int * server_fd, Response * response, Request request);
    
    // encapsulated socket functions
    void init(const char * hostname, const char * port, struct addrinfo * host_info, struct addrinfo ** host_info_list, int type);
    void createSocket(int * fd, struct addrinfo ** host_info_list, int type);
    void connect(int * fd, struct addrinfo ** host_info_list);
    void bind(int * fd, struct addrinfo ** host_info_list);
    void listen(int * fd);
    void accept(int * socket_fd, int * connect_fd, sockaddr_storage * accept_socket_addr);
};

#endif