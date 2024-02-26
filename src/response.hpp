#ifndef _RESPONSE_HPP_
#define _RESPONSE_HPP_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

class Response {
public:
    std::string httpResponse;
    std::string responseHeader;
    std::string responseBody;
    std::string date;
    int status;
    int max_age;
    bool isChunked;
    int contentLength;
    std::string Etag;
    std::string LastModified;
    std::string Expires;
    int cache_mode;

    Response();
    Response(std::string httpResponse);
    Response(const Response & rhs);
    Response & operator=(const Response & rhs);
    ~Response();
    void setResponseLine(std::string line);
    void setDate(std::string line);
    void setStatus(std::string line);
    void setMaxAge(std::string line);
    void setTransferEncoding(std::string line);
    void setCacheControl(std::string line);
    void setContentLength(std::string line);
    void setExpire(std::string line);
    void setEtag(std::string line);
    void setLastModified(std::string line);
    void seperateHeaderAndBody(std::string httpResponse);
    void addResponseBody(std::vector<char> & responseChunkedBody);
    void addResponseBody(std::string responseStringBody);
    void parse();
};

#endif