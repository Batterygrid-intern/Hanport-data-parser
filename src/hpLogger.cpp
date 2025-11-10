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
    const char* timestamp = getTimeStamp();

    std::ostringstream logEntry;
    logEntry << "[" << timestamp << "] "
             << levelToString(level) << ": " << message
             << std::endl;

    if (logFile_.is_open()) {
        logFile_ << logEntry.str();
        logFile_.flush();
    }
}


char* hpLogger::getTimeStamp(){
    time_t now = time(0);
    tm* timeInfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp,sizeof(timestamp), "%Y-%m-%d %H:%M:%S",timeInfo);
    return timestamp;
}