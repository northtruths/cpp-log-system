#pragma once
#include "log_level.hpp"
#include <sstream>

namespace log
{
    class Logger;

    class LogStream
    {
    public:
        LogStream(Logger& owner, Level lv, const char* file, int line);
        ~LogStream();

        LogStream& operator<<(const char* content)
        {
            oss_ << content;
            return *this;
        }

    private:
        Logger& owner_;
        Level lv_;
        const char* file_;
        int line_;
        std::ostringstream oss_;
    };
}