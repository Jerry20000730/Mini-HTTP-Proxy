#include "log.hpp"
#include "util.hpp"

Log::Log() : filepath(LOGFILE_DEFAULT), logstream(std::ofstream(LOGFILE_DEFAULT)) {}
Log::Log(std::string filepath) : filepath(filepath), logstream(std::ofstream(filepath.c_str())) {}
Log::Log(const Log & rhs) : filepath(rhs.filepath), logstream(std::ofstream(filepath.c_str())) {}
Log & Log::operator=(const Log & rhs) {
    if (this != &rhs) {
        filepath = rhs.filepath;
        logstream = std::ofstream(rhs.filepath.c_str());
    }
    return *this;
}
Log::~Log() {}
std::string Log::getCurrentTime() {
    std::time_t curTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&curTime);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", localTime);
    return std::string(buffer);
}
void Log::write(std::string msg) {
    std::lock_guard<std::mutex> lock(mutex_log);
    logstream << msg << std::endl;
}
void Log::logRequest(int id, std::string from_ip, std::string request) {
    std::ostringstream ss;
    ss << id << ": " << "\"" << request << "\" from " << from_ip << " @ " << getCurrentTime();
    write(ss.str());
}
void Log::logRequestInCache(int id, int cache_mode, std::string expired_time) {
    std::ostringstream ss;
    switch (cache_mode) {
        case CACHE_NOEXIST:
            ss << id << ": not in cache";
            break;
        case CACHE_EXIST_EXPIRED:
            ss << id << ": in cache, but expired at " << expired_time;
            break;
        case CACHE_EXIST_NONVALID:
            ss << id << ": in cache, requires validation";
            break;
        case CACHE_EXIST_VALID:
            ss << id << ": in cache, valid";
            break;
    }
    write(ss.str());
}
void Log::logResponseInCache(int id, int cache_mode, std::string reason, std::string expired_time) {
    std::ostringstream ss;
    switch (cache_mode) {
        case CACHE_NOT_CACHEABLE:
            ss << id << ": not cacheable because " << reason;
            break;
        case CACHE_WILL_EXPIRE:
            ss << id << ": cached, expires at " << expired_time;
            break;
        case CACHE_REQUIRE_REVALIDATION:
            ss << id << ": cached, but requires re-validation";
            break;
    }
    write(ss.str());
}
void Log::logContactToServerAboutRequest(int id, std::string to_hostname, std::string request) {
    std::ostringstream ss;
    ss << id << ": " << REQUEST_TAG << " \"" << request << "\" " << "from " << to_hostname;
    write(ss.str());
}
void Log::logResponseFromServer(int id, std::string to_hostname, std::string response) {
    std::ostringstream ss;
    ss << id << ": " << RECEIVE_TAG << " \"" << response << "\" " << "from " << to_hostname;
    write(ss.str());
}
void Log::logRespondingToClient(int id, std::string response) {
    std::ostringstream ss;
    ss << id << ": " << RESPOND_TAG << " \"" << response << "\"";
    write(ss.str());
}
void Log::logNote(int id, std::string note_content) {
    std::ostringstream ss;
    ss << id << ": " << NOTE_TAG << " " << note_content;
    write(ss.str());
}
void Log::logWarning(int id, std::string warning_content) {
    std::ostringstream ss;
    if (id == -1) {
        ss << "(no-id): " << WARNING_TAG << " " << warning_content;
    } else {
        ss << id << ": " << WARNING_TAG << " " << warning_content;
    }
    write(ss.str());
}
void Log::logError(int id, std::string error_content) {
    std::ostringstream ss;
    if (id == -1) {
        ss << "(no-id): " << ERROR_TAG << " " << error_content;
    } else {
        ss << id << ": " << ERROR_TAG << " " << error_content;
    }
    write(ss.str());
}
