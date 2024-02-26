#include "response.hpp"
#include "util.hpp"

Response::Response() : status(-1), max_age(-1), isChunked(false), contentLength(-1), cache_mode(CACHE_NORMAL) {}
Response::Response(std::string httpResponse) : httpResponse(httpResponse), status(-1), max_age(-1), isChunked(false), contentLength(-1), cache_mode(CACHE_NORMAL) {}
Response::Response(const Response & rhs) : httpResponse(rhs.httpResponse), responseHeader(rhs.responseHeader), responseBody(rhs.responseBody), date(rhs.date), status(rhs.status), max_age(rhs.max_age), isChunked(rhs.isChunked), contentLength(rhs.contentLength), Etag(rhs.Etag), LastModified(rhs.LastModified), Expires(rhs.Expires), cache_mode(rhs.cache_mode) {}
Response & Response::operator=(const Response & rhs) {
    if (this != &rhs) {
        httpResponse = rhs.httpResponse;
        responseHeader = rhs.responseHeader;
        responseBody = rhs.responseBody;
        date = rhs.date;
        status = rhs.status;
        max_age = rhs.status;
        isChunked = rhs.isChunked;
        contentLength = rhs.contentLength;
        Etag = rhs.Etag;
        LastModified = rhs.LastModified;
        Expires = rhs.Expires;
        cache_mode = rhs.cache_mode;
    }
    return *this;
}
Response::~Response() {}
void Response::seperateHeaderAndBody(std::string httpResponse) {
    size_t sep = httpResponse.find("\r\n\r\n");
    responseHeader = httpResponse.substr(0, sep);
    responseBody = httpResponse.substr(sep+4);
}
void Response::parse() {
    seperateHeaderAndBody(httpResponse);
    std::istringstream stream(responseHeader);
    std::string line;
    bool firstLine = true;

    while(getline(stream, line)){
        if (!line.empty() && line.back() == '\r'){ 
            line.pop_back();
        }
        if (firstLine){
            setStatus(line);
            firstLine = false;
        }
        if (line.substr(0, strlen(DATE_TAG)) == DATE_TAG) {
            setDate(line);
        }
        if (line.substr(0, strlen(EXPIRE_TAG)) == EXPIRE_TAG) {
            setExpire(line);
        }
        if (line.substr(0, strlen(ETAG_TAG)) == ETAG_TAG) {
            setEtag(line);
        }
        if (line.substr(0, strlen(LAST_MODIFIED_TAG)) == LAST_MODIFIED_TAG) {
            setLastModified(line);
        }
        if (line.substr(0, strlen(CACHE_CONTROL_TAG)) == CACHE_CONTROL_TAG) {
            setCacheControl(line);
        }
        if (line.substr(0, strlen(TRANSFER_ENCODING_TAG)) == TRANSFER_ENCODING_TAG) {
            setTransferEncoding(line);
        }
        if (line.substr(0, strlen(CONTENT_LENGTH_TAG)) == CONTENT_LENGTH_TAG) {
            setContentLength(line);
        }
    }
}
void Response::setDate(std::string line) {
    date = line.substr(line.find(":")+2);
}
void Response::setStatus(std::string line) {
    std::istringstream stream(line);
    std::string protocol;
    std::string status_code_str;
    std::string status_str;
    getline(stream, protocol, ' ');
    getline(stream, status_code_str, ' ');
    getline(stream, status_str, ' ');
    status = atoi(status_code_str.c_str());
}
void Response::setTransferEncoding(std::string line) {
    if (line.find(ENCODING_CHUNKED) != std::string::npos) {
        isChunked = true;
    }
}
void Response::setCacheControl(std::string line) {
    // max-age
    if (size_t pos = line.find(RESPONSE_CACHE_DIRECTIVE_MAXAGE) != std::string::npos) {
        if (size_t sep = line.substr(pos+2).find(",") != std::string::npos) {
            max_age = atoi(line.substr(pos+2, sep-pos-1).c_str());
        } else {
            max_age = atoi(line.substr(pos+2).c_str());
        }
    } else if (size_t pos = line.find(RESPONSE_CACHE_DIRECTIVE_SMAXAGE) != std::string::npos) {
        if (size_t sep = line.substr(pos+2).find(",") != std::string::npos) {
            max_age = atoi(line.substr(pos+2, sep-pos-1).c_str());
        } else {
            max_age = atoi(line.substr(pos+2).c_str());
        }
    }

    // no-store/no-cache/must-revalidate/proxy-revalidate/private
    if (line.find(RESPONSE_CACHE_DIRECTIVE_NOSTORE) != std::string::npos) {
        cache_mode = CACHE_NO_STORE;
    } else if (line.find(RESPONSE_CACHE_DIRECTIVE_PRIVATE) != std::string::npos) {
        cache_mode = CACHE_NO_STORE;
    } else if (line.find(RESPONSE_CACHE_DIRECTIVE_NOCACHE) != std::string::npos) {
        cache_mode = CACHE_MUST_REVALIDATE;
    } else if (line.find(RESPONSE_CACHE_DIRECTIVE_MUST_REVALIDATE) != std::string::npos) {
        if (max_age == 0) {
            cache_mode = CACHE_MUST_REVALIDATE;
        }
    } else if (line.find(RESPONSE_CACHE_DIRECTIVE_PROXY_REVALIDATE) != std::string::npos) {
        if (max_age == 0) {
            cache_mode = CACHE_MUST_REVALIDATE;
        }
    }
}
void Response::setEtag(std::string line) {
    Etag = line.substr(line.find(":")+2);
}
void Response::setExpire(std::string line) {
    Expires = line.substr(line.find(":")+2);
}
void Response::setLastModified(std::string line) {
    LastModified = line.substr(line.find(":")+2);
}
void Response::setContentLength(std::string line) {
    contentLength = atoi(line.substr(line.find(":")+2).c_str());
}
void Response::addResponseBody(std::vector<char> & responseChunkedBody) {
    responseBody += responseChunkedBody.data();
}
void Response::addResponseBody(std::string responseStringBody) {
    responseBody += responseStringBody;
}