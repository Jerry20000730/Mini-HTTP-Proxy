#ifndef _PROXY_HPP_
#define _PROXY_HPP_

#include <netdb.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <pthread.h>

#include "cache.hpp"
#include "log.hpp"

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
    static LRUCache * cache;
    static Log * log;

    // rule of three
    Proxy(int port);
    Proxy(const Proxy & rhs);
    Proxy & operator=(const Proxy & rhs);
    ~Proxy();

    // main function
    static void * task(void * info);
    void run();

    // server start function
    static int initServer(const char * port);
    static void acceptConnection(int * socket_fd, int * connect_fd, std::string * ip);

    // client start function
    static int initClient(const char * hostname, const char * port);

    // request / response send/recv functions
    static std::string recvClientRequest(int * connect_fd);
    static void sendRequestToServer(int * socket_fd, std::string request);
    static void sendResponseToClient(int * socket_fd, std::vector<char> & response);
    static void recvServerResponse(int * server_fd, int * client_fd, Response * response);
    static std::vector<char> recvServerChunkedResponse(int * server_fd, int * client_fd);
    
    // process request
    static void processConnect(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id);
    static void processGet(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id);
    static void processPost(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id); 
    
    // encapsulated socket functions
    static void init(const char * hostname, const char * port, struct addrinfo * host_info, struct addrinfo ** host_info_list, int type);
    static void createSocket(int * fd, struct addrinfo ** host_info_list, int type);
    static void connect(int * fd, struct addrinfo ** host_info_list);
    static void bind(int * fd, struct addrinfo ** host_info_list);
    static void listen(int * fd);
    static void accept(int * socket_fd, int * connect_fd, sockaddr_storage * accept_socket_addr);
};

#endif