#include "EmbedLog/EmbedLog.hpp"

namespace EmbedLog
{
    EmbedLog::EmbedLog(OpenFunction openFunc, CloseFunction closeFunc, PrintFunction printFunc, MicrosecondFunction microsecondFunc, LogLevel logLevel)
        : openFunc(openFunc), closeFunc(closeFunc), printFunc(printFunc), microsecondFunc(microsecondFunc), logLevel(logLevel)
    {
        openFunc();
    }

    EmbedLog::~EmbedLog()
    {
        closeFunc();
    }

    bool EmbedLog::open()
    {
        isOpen = true;
        return openFunc();
    }

    bool EmbedLog::close()
    {
        isOpen = false;
        return closeFunc();
    }

    void EmbedLog::print(LogLevel level, const std::string& message)
    {
        std::string logLevelString;
        switch (level)
        {
        case LogLevel::INFO:
            logLevelString = "INFO";
            break;
        case LogLevel::WARNING:
            logLevelString = "WARNING";
            break;
        case LogLevel::ERROR:
            logLevelString = "ERROR";
            break;
        case LogLevel::DEBUG:
            logLevelString = "DEBUG";
            break;
        }

        printFunc(getTimestamp() + " [" + logLevelString + "] " + message);
    }

    void EmbedLog::setLogLevel(LogLevel level)
    {
        logLevel = level;
    }

    std::string EmbedLog::getTimestamp()
    {
        long microseconds = microsecondFunc();
        long seconds = microseconds / 1000000;
        long minutes = seconds / 60;
        long hours = minutes / 60;

        int hour = hours % 24;
        int minute = minutes % 60;
        int second = seconds % 60;
        int microsec = microseconds % 1000000;

        // Format the timestamp as HH:MM:SS:Microseconds
        char timestamp[30];
        snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d:%06d", hour, minute, second, microsec);
        
        return std::string(timestamp);
    }
}
