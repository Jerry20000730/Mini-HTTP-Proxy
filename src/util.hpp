#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <exception>
#include <fstream>
#include <vector>

#include "log.hpp"

enum CONNECT_TYPE {
    SERVER = 1,
    CLIENT = 2,
};

enum CACHE_MODE {
    CACHE_NOEXIST = 10,
    CACHE_EXIST_EXPIRED = 11,
    CACHE_EXIST_NONVALID = 12,
    CACHE_EXIST_VALID = 13,
};

/**
 * CACHE_NORMAL: revalidate with server if the content is stale
 * CACHE_MUST_REVALIDATE: no matter the content is fresh or stale, if it is in cache, revalidate
 * CACHE_IMMUTABLE: no need to revalidate - the content is static and do not need to update
 * CACHE_NO_STORE: do not store in the cache
*/
enum CACHE_CONTROL_POLOCY {
    CACHE_NORMAL = 20,
    CACHE_MUST_REVALIDATE = 21,
    CACHE_IMMUTABLE = 22,
    CACHE_NO_STORE = 23,
};

/* response tag */
const char * const HTTP_RESPONSE_200 = "HTTP/1.1 200 OK\r\n\r\n";
const char * const HTTP_RESPONSE_200_TAILED = "HTTP/1.1 200 OK";
const char * const HTTP_RESPONSE_400 = "HTTP/1.1 400 Bad Request\r\n\r\n";
const char * const HTTP_RESPONSE_400_TAILED = "HTTP/1.1 400 Bad Request";

/* request tag */
const char * const HOST_TAG = "Host";
const char * const USER_AGENT_TAG = "User-Agent";
const char * const PROXY_CONNECT_TAG = "Proxy-Connection";
const char * const CONNECTION_TAG = "Connection";

/* response tag */
const char * const DATE_TAG = "Date";
const char * const LAST_MODIFIED_TAG = "Last-Modified";
const char * const ETAG_TAG = "ETag";
const char * const EXPIRE_TAG = "Expires";
const char * const TRANSFER_ENCODING_TAG = "Transfer-Encoding";
const char * const CONTENT_LENGTH_TAG = "Content-Length";

/* request connection tag */
const char * const KEEP_ALIVE_TAG = "keep-alive";

/* Transfer-Encoding mode */
const char * const ENCODING_CHUNKED = "chunked";

/* Cache tag in request/response */
const char * const CACHE_CONTROL_TAG = "Cache-Control";

/* Caching directive in request*/
const char * const REQUEST_CACHE_DIRECTIVE_MAXAGE = "max-age";
const char * const REQUEST_CACHE_DIRECTIVE_NOCACHE = "no-cache";
const char * const REQUEST_CACHE_DIRECTIVE_NOSTORE = "no-store";

/* Cacheing directive in response */
const char * const RESPONSE_CACHE_DIRECTIVE_MAXAGE = "max-age";
const char * const RESPONSE_CACHE_DIRECTIVE_NOCACHE = "no-cache"; 
const char * const RESPONSE_CACHE_DIRECTIVE_PRIVATE = "private";
const char * const RESPONSE_CACHE_DIRECTIVE_NOSTORE = "no-store";
const char * const RESPONSE_CACHE_DIRECTIVE_SMAXAGE = "s-maxage";
const char * const RESPONSE_CACHE_DIRECTIVE_MUST_REVALIDATE = "must-revalidate";
const char * const RESPONSE_CACHE_DIRECTIVE_PROXY_REVALIDATE = "proxy-revalidate";

/* log request tag */
const char * const NORMAL_TAG = "";
const char * const REQUEST_TAG = "Requesting";
const char * const RECEIVE_TAG = "Received";
const char * const RESPOND_TAG = "Responding";
const char * const NOTE_TAG = "NOTE";
const char * const ERROR_TAG = "ERROR";
const char * const WARNING_TAG = "WARNING";

/* default logfile path */
const char * const LOGFILE_DEFAULT = "proxy.log";

/* default port number */
const char * const DEFAULT_PORT = "80";

class ThreadSafeIdGenerator {
private:
    int id;
    pthread_mutex_t mutex_id;

public:
    ThreadSafeIdGenerator() : id(139) {
        pthread_mutex_init(&mutex_id, NULL);
    }

    ~ThreadSafeIdGenerator() {
        pthread_mutex_destroy(&mutex_id);
    }

    void increment() {
        pthread_mutex_lock(&mutex_id);
        id++;
        pthread_mutex_unlock(&mutex_id);
    }

    int get_id() {
        increment();
        return id;
    }
};

class NoSuchHostNameException: public std::exception {
private:
    std::string message;
public:
    NoSuchHostNameException(int type) {
        std::string type_str = (type == SERVER) ? "Proxy as server" : "Proxy as client";
        message = "NoSuchHostNameException: " + type_str + " cannot get address info";
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class SocketCreationException: public std::exception {
private:
    std::string message;
public:
    SocketCreationException(int type) {
        std::string type_str = (type == SERVER) ? "Proxy as server" : "Proxy as client";
        message = "SocketCreationException: " + type_str + " cannot create socket properly";
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class SocketConnectException : public std::exception {
private:
    std::string message;
public:
    SocketConnectException(int type) {
        std::string type_str = (type == SERVER) ? "Proxy as server" : "Proxy as client";
        message = "SocketConnectException: " + type_str + " failed to connect to the socket of server";
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class SocketBindException : public std::exception {
private:
    std::string message;
public:
    SocketBindException(int type) {
        std::string type_str = (type == SERVER) ? "Proxy as server" : "Proxy as client";
        message = "SocketBindException: " + type_str + " failed to bind ip/port to socket";
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class SocketListenException : public std::exception {
private:
    std::string message;
public:
    SocketListenException(int type) {
        std::string type_str = (type == SERVER) ? "Proxy as server" : "Proxy as client";
        message = "SocketListenException: " + type_str + " does not listen on socket properly";
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class SocketAcceptException : public std::exception {
private:
    std::string message;
public:
    SocketAcceptException(int type) {
        std::string type_str = (type == SERVER) ? "Proxy as server" : "Proxy as client";
        message = "SocketAcceptException: " + type_str + " cannot accept properly";
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class RecvException : public std::exception {
private:
    std::string message;
public:
    RecvException(int byte_count, int from) {
        if (byte_count == 0) {
            if (from == CLIENT) {
                message = "RecvException: recv from client socket closed";
            } else {
                message = "RecvException: recv from server socket closed";
            }
        } else if (byte_count == -1) {
            if (from == CLIENT) {
                message = "RecvException: cannot accept client socket correctly";
            } else {
                message = "RecvException: cannot accept server socket correctly";
            }
        }
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

class SelectException : public std::exception {
private:
    std::string message;
public:
    SelectException(int rv) {
        if (rv == -1) {
            message = "SelectException: select failed";
        } else if (rv == 0) {
            message = "SelectException: select timeout";
        }
    }
    const char* what() const throw() {
        return message.c_str();
    }
};

std::string getIpaddress(struct sockaddr_storage* addr_storage);
bool endsWith(const std::string& fullString, const std::string& ending);
void printVectorData(const std::vector<char>& data);

#endif