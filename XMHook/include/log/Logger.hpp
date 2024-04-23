#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <ctime>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    _ERROR,
    FATAL
};

class Logger
{
public:
    Logger(const std::string &filename) : file(filename) {}

    void Log(LogLevel level, const char *format, ...)
    {
        // std::string message = std::format(format, args...);
        char buffer[1024]; // 定义一个足够大的缓冲区
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        std::string message(buffer);

        std::lock_guard<std::mutex> lock(mtx);
        std::time_t now = std::time(nullptr);
        std::tm *localTime = std::localtime(&now);
        std::string timeStr = std::asctime(localTime);
        timeStr.pop_back(); // Remove newline character

        std::string levelStr;
        switch (level)
        {
        case LogLevel::DEBUG:
            levelStr = "DEBUG";
            break;
        case LogLevel::INFO:
            levelStr = "INFO";
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            break;
        case LogLevel::_ERROR:
            levelStr = "ERROR";
            break;
        case LogLevel::FATAL:
            levelStr = "FATAL";
            break;
        }

        std::string logMessage = timeStr + " [" + levelStr + "] " + message + "\n";
        std::cout << logMessage;
        file << logMessage;
    }

private:
    std::ofstream file;
    std::mutex mtx;
};
