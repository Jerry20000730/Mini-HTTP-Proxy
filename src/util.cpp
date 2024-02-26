#include "util.hpp"

std::string getIpaddress(struct sockaddr_storage* addr_storage) {
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr_storage;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
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