#include "request.hpp"
#include "util.hpp"

Request::Request(std::string httpRequest) : httpRequest(httpRequest) {}
Request::Request(const Request & rhs) : httpRequest(rhs.httpRequest), method(rhs.method), requestLine(rhs.requestLine), hostname(rhs.hostname), userAgent(rhs.userAgent), url(rhs.url), proxy_connection(rhs.proxy_connection), connection(rhs.connection), port(rhs.port) {}
Request & Request::operator=(const Request & rhs) {
    if (this != &rhs) {
        httpRequest = rhs.httpRequest;
        method = rhs.method;
        requestLine = rhs.requestLine;
        hostname = rhs.hostname;
        userAgent = rhs.userAgent;
        url = rhs.url;
        proxy_connection = rhs.proxy_connection;
        connection = rhs.connection;
        port = rhs.port;
    }
    return (*this);
}
Request::~Request() {}

void Request::parse(){
    std::istringstream stream(httpRequest);
    std::string line;
    bool firstLine = true;

    while(getline(stream, line)){
        if (!line.empty()&&line.back() == '\r'){ 
            //remove \r
            line.pop_back();
        }
        if (firstLine){
            setRequestLine(line);
            firstLine = false;
        }
        else if (line.substr(0, strlen(HOST_TAG)) == HOST_TAG){
            setHostnameAndPort(line);
        }
        else if (line.substr(0, strlen(USER_AGENT_TAG)) == USER_AGENT_TAG){
            setUserAgent(line);
        }
        else if (line.substr(0, strlen(PROXY_CONNECT_TAG)) == PROXY_CONNECT_TAG){
            setProxyConnection(line);
        }
        else if (line.substr(0, strlen(CONNECTION_TAG)) == CONNECTION_TAG) {
            setConnection(line);
        }
    }
}

void Request::setRequestLine(std::string line) {
    requestLine = line;
    std::istringstream stream(line);
    getline(stream, method, ' '); 
    getline(stream, url, ' ');
}

void Request::setHostnameAndPort(std::string line){
    size_t pos1 = line.find(":");
    size_t pos2 = line.find(":", pos1+1);
    if (pos2 != std::string::npos) {
        hostname = line.substr(pos1+2, pos2 - (pos1 + 2));
        port = line.substr(pos2+1);
    } else {
        hostname = line.substr(pos1+2);
    }
}

void Request::setUserAgent(std::string line){ 
    userAgent = line.substr(line.find(":") + 2);
}

void Request::setConnection(std::string line){
    connection = line.substr(line.find(":") + 2);
}

void Request::setProxyConnection(std::string line){
    proxy_connection = line.substr(line.find(":") + 2);
}