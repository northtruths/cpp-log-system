// log_level.hpp
#pragma once
namespace log
{
    enum class Level : int
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LEVELS
    };

    inline char *to_string(Level lv)
    {
        switch (lv)
        {
        case Level::TRACE:
            return "TRACE";
        case Level::DEBUG:
            return "DEBUG";
        case Level::INFO:
            return "INFO";
        case Level::WARN:
            return "WARN";
        case Level::ERROR:
            return "ERROR";
        case Level::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }
} //namespace log