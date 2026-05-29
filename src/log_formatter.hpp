// log_formatter.hpp
#pragma once
#include "log_message.hpp"
#include <string>

namespace log
{

    class Formatter
    {
    public:
        virtual ~Formatter() = default;
        virtual void format(LogMsg &msg) = 0;
    };

    std::unique_ptr<Formatter> make_default_formatter();
} // namespace log