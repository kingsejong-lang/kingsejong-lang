#pragma once

#include <string>
#include <mutex>
#include <type_traits>
#include <string_view>

namespace kingsejong {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static void setLevel(LogLevel level);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

    // For formatted output (simple version)
    template<typename... Args>
    static void debug(const std::string& format, Args... args) {
        if (currentLevel <= LogLevel::DEBUG) {
            log(LogLevel::DEBUG, formatString(format, args...));
        }
    }

    // Helper for string formatting - replaces {} placeholders with arguments
    static std::string formatString(const std::string& format) {
        return format;
    }

    template<typename T, typename... Args>
    static std::string formatString(const std::string& format, T value, Args... args) {
        size_t pos = format.find("{}");
        if (pos == std::string::npos) {
            return format;
        }
        
        std::string result = format.substr(0, pos);
        
        // Handle different types
        if constexpr (std::is_same_v<T, std::string> || 
                      std::is_same_v<T, const char*> ||
                      std::is_same_v<T, char*>) {
            result += value;
        } else if constexpr (std::is_same_v<T, std::string_view>) {
            result += std::string(value);
        } else if constexpr (std::is_arithmetic_v<T>) {
            result += std::to_string(value);
        } else {
            // Fallback for other types
            result += std::string(value);
        }
        
        result += format.substr(pos + 2);
        
        return formatString(result, args...);
    }

private:
    static LogLevel currentLevel;
    static std::mutex logMutex;

    static void log(LogLevel level, const std::string& message);
    static std::string getLevelString(LogLevel level);
};

} // namespace kingsejong
