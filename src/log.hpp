#ifndef _LOG_HPP_
#define _LOG_HPP_

#include<iostream>
#include<fstream>
#include<ctime>

/* no-id tag */
const int NO_ID = -10;

class Log {
private:
    std::string filepath;
    std::ofstream logstream;
public:
    Log();
    Log(std::string filepath);
    Log(const Log & rhs);
    Log & operator=(const Log & rhs);
    ~Log();
    std::string getCurrentTime();
    void write(std::string msg);
    void logRequest(int id, std::string from_ip, std::string request);
    void logCache(int id, int cache_mode, std::string expired_time);
    void logContactToServerAboutRequest(int id, std::string to_hostname, std::string request);
    void logResponseFromServer(int id, std::string to_hostname, std::string response);
    void logRespondingToClient(int id, std::string response);
    void logNote(int id, std::string note_content);
    void logWarning(int id, std::string warning_content);
    void logError(int id, std::string error_content);
};

#endif