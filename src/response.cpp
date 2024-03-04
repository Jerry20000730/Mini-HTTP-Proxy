#include "response.hpp"
#include "util.hpp"

Response::Response() : status(-1), max_age(-1), isChunked(false), contentLength(-1), cache_mode(CACHE_NORMAL) {}
Response::Response(std::string httpResponse) : httpResponse(httpResponse), status(-1), max_age(-1), isChunked(false), contentLength(-1), cache_mode(CACHE_NORMAL) {}
Response::Response(const Response & rhs) : httpResponse(rhs.httpResponse), responseLine(rhs.responseLine), responseHeader(rhs.responseHeader), responseBody(rhs.responseBody), date(rhs.date), status(rhs.status), max_age(rhs.max_age), isChunked(rhs.isChunked), contentLength(rhs.contentLength), CacheControl(rhs.CacheControl), Etag(rhs.Etag), LastModified(rhs.LastModified), Expires(rhs.Expires), cache_mode(rhs.cache_mode), expireTime(rhs.expireTime) {}
Response & Response::operator=(const Response & rhs) {
    if (this != &rhs) {
        httpResponse = rhs.httpResponse;
        responseLine = rhs.responseLine;
        responseHeader = rhs.responseHeader;
        responseBody = rhs.responseBody;
        date = rhs.date;
        status = rhs.status;
        max_age = rhs.status;
        isChunked = rhs.isChunked;
        contentLength = rhs.contentLength;
        CacheControl = rhs.CacheControl;
        Etag = rhs.Etag;
        LastModified = rhs.LastModified;
        Expires = rhs.Expires;
        cache_mode = rhs.cache_mode;
        expireTime = rhs.expireTime;
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
    try {
        seperateHeaderAndBody(httpResponse);
        std::istringstream stream(responseHeader);
        std::string line;
        bool firstLine = true;

        while(getline(stream, line)) {
            
            if (!line.empty() && line.back() == '\r'){ 
                line.pop_back();
            }
            if (firstLine){
                setResponseLine(line);
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

        setExpiredTime();
    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
    }
}
void Response::setResponseLine(std::string line) {
    responseLine = line;
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
    CacheControl = line;

    // max-age
    size_t pos1 = line.find(RESPONSE_CACHE_DIRECTIVE_MAXAGE);
    if (pos1 != std::string::npos) {
        size_t sep1 = line.substr(pos1+2).find(",");
        if (sep1 != std::string::npos) {
            max_age = atoi(line.substr(pos1+strlen(RESPONSE_CACHE_DIRECTIVE_MAXAGE)+1, sep1-pos1-strlen(RESPONSE_CACHE_DIRECTIVE_MAXAGE)-1).c_str());
        } else {
            max_age = atoi(line.substr(pos1+strlen(RESPONSE_CACHE_DIRECTIVE_MAXAGE)+1).c_str());
        }
    }
    size_t pos2 = line.find(RESPONSE_CACHE_DIRECTIVE_SMAXAGE);
    if (pos2 != std::string::npos) {
        size_t sep2 = line.substr(pos2+2).find(",");
        if (sep2 != std::string::npos) {
            max_age = atoi(line.substr(pos2+strlen(RESPONSE_CACHE_DIRECTIVE_SMAXAGE)+1, sep2-pos2-strlen(RESPONSE_CACHE_DIRECTIVE_SMAXAGE)-1).c_str());
        } else {
            max_age = atoi(line.substr(pos2+strlen(RESPONSE_CACHE_DIRECTIVE_SMAXAGE)+1).c_str());
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
void Response::addResponseBody(std::vector<char> * responseChunkedBody) {
    if (responseChunkedBody != nullptr || responseBody.size() != 0) {
        httpResponse = responseHeader + "\r\n\r\n" + responseBody;
        responseBody.append(responseChunkedBody->data(), responseChunkedBody->size());
        httpResponse.append(responseChunkedBody->data(), responseChunkedBody->size());
    }
}
void Response::addResponseBody(std::string responseStringBody) {
    responseBody += responseStringBody;
    httpResponse = responseHeader + "\r\n\r\n" + responseBody;
}
bool Response::isValid() {
    if (!httpResponse.empty()) {
        if (httpResponse.find("\r\n\r\n") == std::string::npos) {
            return false;
        }
        return true;
    }
    return false;
}
long long Response::timeDifferenceInSeconds(std::string last_modified, std::string now) {
    std::tm tm_last_modified = {};
    std::tm tm_now = {};
    std::istringstream ss_last_modified(last_modified);
    ss_last_modified >> std::get_time(&tm_last_modified, "%a, %d %b %Y %H:%M:%S GMT");
    
    // Parse now date
    std::istringstream ss_now(now);
    ss_now >> std::get_time(&tm_now, "%a, %d %b %Y %H:%M:%S GMT");
    
    // Convert to time_t
    auto time_t_last_modified = std::mktime(&tm_last_modified);
    auto time_t_now = std::mktime(&tm_now);
    
    // Convert to chrono::system_clock::time_point
    auto tp_last_modified = std::chrono::system_clock::from_time_t(time_t_last_modified);
    auto tp_now = std::chrono::system_clock::from_time_t(time_t_now);
    
    // Calculate difference
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(tp_now - tp_last_modified).count();
    return duration;
}
void Response::setExpiredTime() {
    if (max_age != -1) {
        std::tm tm = {};
        std::istringstream ss(date);
        std::stringstream res;

        ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        auto time_c = std::mktime(&tm);
        std::chrono::system_clock::time_point responseDate_chrono = std::chrono::system_clock::from_time_t(time_c);
        std::chrono::system_clock::time_point expiredTime_chrono = responseDate_chrono + std::chrono::seconds(max_age);
        
        std::time_t finalExpiredTime = std::chrono::system_clock::to_time_t(expiredTime_chrono);
        std::tm* gmtTime = std::gmtime(&finalExpiredTime);
        res << std::put_time(gmtTime, "%a, %d %b %Y %H:%M:%S GMT");

        expireTime = res.str();
        return;
    }
    if (!Expires.empty()) {
        expireTime = Expires;
        return;
    }
    if (cache_mode == CACHE_MUST_REVALIDATE) {
        expireTime = date;
    }
    
    // heuristic caching
    // add 10% of time difference to expired time
    if (cache_mode != CACHE_NO_STORE && !LastModified.empty() && !date.empty()) {
        std::tm tm = {};
        std::istringstream ss(date);
        std::stringstream res;

        ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        auto time_c = std::mktime(&tm);
        std::chrono::system_clock::time_point responseDate_chrono = std::chrono::system_clock::from_time_t(time_c);
        
        // calculate seconds for 0.1 of time difference
        auto time_difference = timeDifferenceInSeconds(LastModified, date);
        auto seconds_to_add = static_cast<int>(0.1 * time_difference);

        std::chrono::system_clock::time_point expiredTime_chrono = responseDate_chrono + std::chrono::seconds(seconds_to_add);

        std::time_t finalExpiredTime = std::chrono::system_clock::to_time_t(expiredTime_chrono);
        std::tm* gmtTime = std::gmtime(&finalExpiredTime);
        res << std::put_time(gmtTime, "%a, %d %b %Y %H:%M:%S GMT");

        expireTime = res.str();
    }
}