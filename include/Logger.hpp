#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <mutex>

class Logger {
public:
    // Initialize logger with log file path
    static void init(const std::string& logFilePath);

    // Log an error message with timestamp and process name
    static void logError(const std::string& process, const std::string& errorMessage);

    // Log latest sent message
    static void logLatestMessage(const std::string& message);

    // Get the latest message that was logged
    static std::string getLatestMessage();

private:
    static std::string getCurrentTimestamp();
    static std::string logFilePath;
    static std::string latestMessage;
    static std::mutex logMutex;  // For thread safety
};