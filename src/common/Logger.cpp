#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace kingsejong {

LogLevel Logger::currentLevel = LogLevel::INFO;
std::mutex Logger::logMutex;

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentLevel = level;
}

void Logger::debug(const std::string& message) {
    if (currentLevel <= LogLevel::DEBUG) {
        log(LogLevel::DEBUG, message);
    }
}

void Logger::info(const std::string& message) {
    if (currentLevel <= LogLevel::INFO) {
        log(LogLevel::INFO, message);
    }
}

void Logger::warn(const std::string& message) {
    if (currentLevel <= LogLevel::WARN) {
        log(LogLevel::WARN, message);
    }
}

void Logger::error(const std::string& message) {
    if (currentLevel <= LogLevel::ERROR) {
        log(LogLevel::ERROR, message);
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);

    // Timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif

    std::ostream& out = (level == LogLevel::ERROR) ? std::cerr : std::cout;

    out << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
        << "[" << getLevelString(level) << "] "
        << message << std::endl;
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default:              return "UNKNOWN";
    }
}

} // namespace kingsejong
