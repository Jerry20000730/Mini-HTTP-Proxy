#ifndef _REQUEST_HPP_
#define _REQUEST_HPP_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>

class Request{
public:
    std::string httpRequest;
    std::string method;
    std::string requestLine;
    std::string hostname;
    std::string userAgent;
    std::string url;
    std::string proxy_connection;
    std::string connection;
    std::string port;

    Request(std::string httpRequest);
    Request(const Request & rhs);
    Request & operator=(const Request & rhs);
    ~Request();
    void setRequestLine(std::string line);
    void setHostnameAndPort(std::string line);
    void setUserAgent(std::string line);
    void setProxyConnection(std::string line);
    void setConnection(std::string line);
    void setKeepAlive(std::string line);
    void parse();
};
#endif