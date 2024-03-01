#include "util.hpp"

std::string getIpaddress(struct sockaddr_storage* addr_storage) {
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr_storage;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
    return std::string(ipstr);
}

std::string getIpaddress(const char * hostname) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, NULL, &hints, &res);
    if (status != 0) {
        throw NoSuchHostNameException(CLIENT);
    }
    
    p = res;
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)(p->ai_addr);

    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);

    freeaddrinfo(res);
    return std::string(ipstr);
}

bool endsWith(const std::string& fullString, const std::string& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void printVectorData(const std::vector<char>& data) {
    std::cout.write(data.data(), data.size());
    std::cout << std::endl;
}