#include "Logger.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>

std::string Logger::logFilePath;
std::string Logger::latestMessage;
std::mutex Logger::logMutex;

void Logger::init(const std::string& logPath) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFilePath = logPath;
    
    // Create directory for log file if it doesn't exist
    std::filesystem::create_directories(std::filesystem::path(logPath).parent_path());
    
    // Create or verify the log file is writable
    std::ofstream logFile(logPath, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to initialize log file: " << logPath << std::endl;
    }
}

void Logger::logError(const std::string& process, const std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = getCurrentTimestamp();
    
    std::ofstream logFile(logFilePath, std::ios::app);  // Append mode
    if (logFile.is_open()) {
        logFile << "[" << timestamp << "][" << process << "] ERROR: " << errorMessage << std::endl;
        
        // Also log the latest message if it exists
        if (!latestMessage.empty()) {
            logFile << "[" << timestamp << "][" << process << "] Last sent message: " << latestMessage << std::endl;
        }
        
        logFile.close();
    } else {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}

void Logger::logLatestMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    latestMessage = message;
    std::string timestamp = getCurrentTimestamp();
    
    std::ofstream logFile(logFilePath, std::ios::app);  // Append mode
    if (logFile.is_open()) {
        logFile << "[" << timestamp << "][MESSAGE] " << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}

std::string Logger::getLatestMessage() {
    std::lock_guard<std::mutex> lock(logMutex);
    return latestMessage;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}