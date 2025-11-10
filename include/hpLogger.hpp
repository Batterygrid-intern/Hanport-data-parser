#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>

//Loggin  levels DEBUG INFO WARN ERROR CRITICAL
//Final destiantion where to outpput the loggs
//Context and timestamps chronological context with timestamps
//Setup dynamic flexibility to change log levels and destinations at runtime
enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};


class hpLogger{
    public:
        hpLogger(const std::string& filepath);
        void log(LogLevel level, const std::string& message);
        std::string getTimeStamp() const;
    private:
        std::ofstream logFile_;
        std::string levelToString(LogLevel level);

};