#include "hpLogger.hpp"


//construct object and open filepath in append mode
hpLogger::hpLogger(const std::string& filepath) {
    logFile_.open(filepath, std::ios::app);
    if (!logFile_.is_open()) {
        std::cerr << "Failed to open log file: " << filepath << std::endl;
    }
}

void hpLogger::log(LogLevel level, const std::string& message){
    //get time stamp for loggmessage
    std::string timestamp = getTimeStamp();
    //the entry / message to append to file
    std::ostringstream logEntry;
    logEntry << "[" << timestamp << "] "
             << levelToString(level) << ": " << message
             << std::endl;
    //append message to file
    if (logFile_.is_open()) {
        logFile_ << logEntry.str();
        logFile_.flush();
    }
    else {
        std::cerr << "Failed to append message to file" << std::endl;
    }
}


std::string hpLogger::getTimeStamp() const {
    time_t now = time(0);
    tm timeInfo;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&timeInfo, &now);
#else
    localtime_r(&now, &timeInfo);
#endif
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeInfo);
    return std::string(timestamp);
}

std::string hpLogger::levelToString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        case CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}