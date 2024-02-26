#include "proxy.hpp"
#include "util.hpp"
#include "request.hpp"
#include "response.hpp"

// static member initialization
LRUCache* Proxy::cache = new LRUCache(50);
Log * Proxy::log = new Log();
pthread_mutex_t mutex_info = PTHREAD_MUTEX_INITIALIZER;

// constructor
Proxy::Proxy(int port) : port(port) {}
// copy constructor
Proxy::Proxy(const Proxy & rhs) : port(rhs.port) {}
// operator=
Proxy & Proxy::operator=(const Proxy & rhs) {
    if (this != &rhs) {
        port = rhs.port;
    }
    return (*this);
}
// destructor
Proxy::~Proxy() {}

int Proxy::initServer(const char * port) {
    int server_socket_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;

    init("0.0.0.0", port, &host_info, &host_info_list, SERVER);
    createSocket(&server_socket_fd, &host_info_list, SERVER);
    bind(&server_socket_fd, &host_info_list);
    listen(&server_socket_fd);

    freeaddrinfo(host_info_list);
    return server_socket_fd;
}

int Proxy::initClient(const char * hostname, const char * port) {
    int client_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;

    if (strlen(port) == 0) {
        port = DEFAULT_PORT;
    }

    try {
        init(hostname, port, &host_info, &host_info_list, CLIENT);
        createSocket(&client_fd, &host_info_list, CLIENT);
        connect(&client_fd, &host_info_list);
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
    }

    freeaddrinfo(host_info_list);
    return client_fd;
}

void * Proxy::task(void * thread_info) {
    info * args = (info*)thread_info;
    thread_local int client_socket_fd;
    try {
        std::string request = recvClientRequest(&args->server_connect_fd);
        if (strlen(request.c_str()) == 0) {
            return NULL;
        }
        Request requestParameter(request);
        requestParameter.parse();
        log->logRequest(args->id, args->ip, requestParameter.requestLine);

        if (requestParameter.method == "CONNECT") {
            client_socket_fd = initClient(requestParameter.hostname.c_str(), requestParameter.port.c_str());
            log->logContactToServerAboutRequest(args->id, requestParameter.hostname, requestParameter.requestLine);
            processConnect(&args->server_connect_fd, &client_socket_fd, &args->ip, &requestParameter.hostname, args->id);
        } else if (requestParameter.method == "GET") {
            client_socket_fd = initClient(requestParameter.hostname.c_str(), requestParameter.port.c_str());
            sendRequestToServer(&client_socket_fd, request);
            log->logContactToServerAboutRequest(args->id, requestParameter.hostname, requestParameter.requestLine);
            processGet(&args->server_connect_fd, &client_socket_fd, &args->ip, &requestParameter.hostname, args->id);      
        } else if (requestParameter.method == "POST") {
            client_socket_fd = initClient(requestParameter.hostname.c_str(), requestParameter.port.c_str());
            sendRequestToServer(&client_socket_fd, request);
            log->logContactToServerAboutRequest(args->id, requestParameter.hostname, requestParameter.requestLine);
            processPost(&args->server_connect_fd, &client_socket_fd, &args->ip, &requestParameter.hostname, args->id);
        } else {
            send(args->server_connect_fd, HTTP_RESPONSE_400, strlen(HTTP_RESPONSE_400)+1, 0);
            log->logResponseFromServer(args->id, requestParameter.hostname.c_str(), HTTP_RESPONSE_400_TAILED);
        }
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
    }

    close(client_socket_fd);
    close(args->server_connect_fd);

    return NULL;
}

void Proxy::run() {
    int server_socket_fd;
    int server_connect_fd;
    std::string ip;
    ThreadSafeIdGenerator * g = new ThreadSafeIdGenerator();

    try {
        server_socket_fd = initServer(std::to_string(port).c_str());
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
    }
    while (true) {
        try {
            pthread_mutex_lock(&mutex_info);
            acceptConnection(&server_socket_fd, &server_connect_fd, &ip);
            pthread_t thread;
            info * task_info = new info();
            task_info->server_socket_fd = server_socket_fd;
            task_info->server_connect_fd = server_connect_fd;
            std::cout << "server_connect_fd created: " << std::to_string(server_connect_fd) << std::endl;
            task_info->ip = ip;
            task_info->id = g->get_id();
            pthread_create(&thread, NULL, task, (void*)task_info);
            pthread_mutex_unlock(&mutex_info);
        } catch (std::exception & e) {
             std::cout << e.what() << std::endl;
        }
    }

    close(server_socket_fd);
}

void Proxy::acceptConnection(int * socket_fd, int * connect_fd, std::string * ip) {
    struct sockaddr_storage accept_socket_addr;
    accept(socket_fd, connect_fd, &accept_socket_addr);
    *ip = getIpaddress(&accept_socket_addr);
}

std::string Proxy::recvClientRequest(int * connect_fd) {
    char buffer[65536];
    memset(buffer, 0, sizeof(buffer));
    int byte_count = recv(*connect_fd, buffer, sizeof(buffer), 0);
    if (byte_count < 0) {
        throw RecvException(byte_count, CLIENT);
    }
    return std::string(buffer);
}

void Proxy::recvServerResponse(int * server_fd, int * client_fd, Response * response) {
    char buffer[65536];
    memset(buffer, 0, sizeof(buffer));
    int byte_count = recv(*server_fd, buffer, sizeof(buffer), 0);
    if (byte_count < 0) {
        throw RecvException(byte_count, CLIENT);
    }

    std::string httpResponse = buffer;
    *response = Response(httpResponse);
    response->parse();

    if (response->isChunked) {
        send(*client_fd, buffer, byte_count, 0);
        std::vector<char> response_chunked = recvServerChunkedResponse(server_fd, client_fd);
        response->addResponseBody(response_chunked);
    } else {
        if (response->contentLength > 65536) {
            std::vector<char> response_chunked = recvServerChunkedResponse(server_fd, client_fd);
            response->addResponseBody(response_chunked);
        } else {
            char buffer[65536];
            memset(buffer, 0, sizeof(buffer));
            recv(*server_fd, buffer, sizeof(buffer), 0);
            response->addResponseBody(buffer);
        }
    }
}

std::vector<char> Proxy::recvServerChunkedResponse(int * server_fd, int * client_fd) {
    std::vector<char> response;
    // Receiving the response
    char buffer[1024];
    memset(buffer, 0, 1024);
    int byte_count = 0;

    // Loop until there's no more data
    while (true) {
        memset(buffer, 0, 1024);
        byte_count = recv(*server_fd, buffer, sizeof(buffer), 0);
        if (byte_count <= 0) {
            break;
        }
        response.insert(response.end(), buffer, buffer + byte_count);
        send(*client_fd, buffer, byte_count, 0);
    }
    return response;
}

void Proxy::sendRequestToServer(int * socket_fd, std::string request) {
    send(*socket_fd, request.c_str(), strlen(request.c_str())+1, 0);
}

void Proxy::sendResponseToClient(int * socket_fd, std::vector<char> & response) {
    send(*socket_fd, response.data(), response.size(), MSG_NOSIGNAL);
}

void Proxy::processGet(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id) {
    Response res;
    recvServerResponse(server_fd, client_fd, &res);
    log->logResponseFromServer(id, *to_hostname, res.responseHeader);

    /* here to judge whether it is need to put in the cache */
    if (res.status == 200 && res.cache_mode != CACHE_NO_STORE) {
        cache->put(*ip, res);
    }
}

void Proxy::processConnect(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id) {
    send(*client_fd, HTTP_RESPONSE_200, strlen(HTTP_RESPONSE_200)+1, 0);
    log->logResponseFromServer(id, *to_hostname, HTTP_RESPONSE_200_TAILED);

    int max_fd = std::max(*client_fd, *server_fd) + 1;
    fd_set readfds;
    struct timeval tv;
    int fd[2] = {*server_fd, *client_fd};

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(*server_fd, &readfds);
        FD_SET(*client_fd, &readfds);

        // set timeout 10.5s
        tv.tv_sec = 10;
        tv.tv_usec = 500000;

        int rv = select(max_fd+1, &readfds, NULL, NULL, &tv);
        if (rv == 0) {
            return;
        }
        if (rv != -1 && rv != 0) {
            for (int i=0; i<2; i++) {
                if (FD_ISSET(fd[i], &readfds)) {
                    char buffer[65536];
                    memset(buffer, 0, 65536);
                    int byte_count_recv = recv(fd[i], buffer, 65536, MSG_NOSIGNAL);
                    if (byte_count_recv <= 0) {
                        return;
                    }
                    int byte_count_send = send(fd[1-i], buffer, byte_count_recv, MSG_NOSIGNAL);
                    if (byte_count_send <= 0) {
                        return;
                    }
                }
            }
        }
    }
}

void Proxy::processPost(int * client_fd, int * server_fd, std::string * ip, std::string * to_hostname, int id){
    std::string response;
    size_t buffer_size = 65536;
    char buffer[buffer_size];
    size_t bytes_received;
    while (true){
        bytes_received = recv(*server_fd, buffer, buffer_size-1, 0);
        if(bytes_received <=0){
            break;
        }
        response.append(buffer, bytes_received);
        if (endsWith(response, "0\r\n\r\n")){
            break;
        }
    }
    send(*client_fd, response.data(), response.length(), 0);
}

void Proxy::init(const char * hostname, const char * port, struct addrinfo * host_info, struct addrinfo ** host_info_list, int type) {
    int status;
    memset(host_info, 0, sizeof(*host_info));
    host_info->ai_family   = AF_UNSPEC;
    host_info->ai_socktype = SOCK_STREAM;
    if (type == SERVER) {
        host_info->ai_flags = AI_PASSIVE;
    }
    status = getaddrinfo(hostname, port, host_info, host_info_list);
    if (status != 0) {
        throw NoSuchHostNameException(type);
    }
}

void Proxy::createSocket(int * fd, struct addrinfo ** host_info_list, int type) {
    *fd = socket((*host_info_list)->ai_family, (*host_info_list)->ai_socktype, (*host_info_list)->ai_protocol);
    if (*fd == -1) {
        throw SocketCreationException(type);
    }
}

void Proxy::connect(int * fd, struct addrinfo ** host_info_list) {
    int status = ::connect(*fd, (*host_info_list)->ai_addr, (*host_info_list)->ai_addrlen);
    if (status == -1) {
        throw SocketConnectException(CLIENT);
    }
}

void Proxy::bind(int * fd, struct addrinfo ** host_info_list) {
    int yes = 1;
    int status = setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = ::bind(*fd, (*host_info_list)->ai_addr, (*host_info_list)->ai_addrlen);
    if (status == -1) {
        throw SocketBindException(SERVER);
    }
}

void Proxy::listen(int * fd) {
    int status = ::listen(*fd, 100);
    if (status == -1) {
        throw SocketListenException(SERVER);
    }
}

void Proxy::accept(int * socket_fd, int * connect_fd, struct sockaddr_storage * accept_socket_addr) {
    socklen_t socket_addr_len = sizeof(accept_socket_addr);
    memset(accept_socket_addr, 0, sizeof(*accept_socket_addr));
    *connect_fd = ::accept(*socket_fd, (struct sockaddr *)accept_socket_addr, &socket_addr_len);
    if (*connect_fd == -1) {
        throw SocketAcceptException(SERVER);
    }
}
