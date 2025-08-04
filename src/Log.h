#pragma once

#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

// Enum to represent log levels
enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger
{
public:
    // Singleton instance getter
    static Logger& shared()
    {
        static Logger instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Logs a message with a given log level
    void log(LogLevel level, const std::string& message)
    {
        // Get current timestamp
        time_t now = time(0);
        tm timeinfo;
        localtime_s(&timeinfo, &now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

        // Create log entry
        std::ostringstream logEntry;
        logEntry << "[" << timestamp << "] "
                 << levelToString(level) << ": " << message
                 << std::endl;

        std::string logString = logEntry.str();

        // Output to log file
        if (ofs.is_open())
        {
            ofs << logString;
            ofs.flush(); // Ensure immediate write to file
        }
    }

    // Convenience methods for different log levels
    void debug(const std::string& message) { log(DEBUG, message); }
    void info(const std::string& message) { log(INFO, message); }
    void warning(const std::string& message) { log(WARNING, message); }
    void error(const std::string& message) { log(ERROR, message); }

private:
    std::ofstream ofs; // File stream for the log file

    // Private constructor: Opens the log file in append mode
    Logger()
    {
        ofs.open("tmp\\log.txt", std::ofstream::trunc);
    }

    // Destructor: Closes the log file
    ~Logger()
    {
        if (ofs.is_open())
            ofs.close();
    }

    // Converts log level to a string for output
    std::string levelToString(LogLevel level)
    {
        switch (level)
        {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }
};

#define log(x) Logger::shared().debug(x)