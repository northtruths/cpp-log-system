// log_formatter.hpp
#pragma once
#include "log_message.hpp"
#include <string>

namespace log {

class Formatter {
public:
    virtual ~Formatter() = default;
    virtual std::string format(const LogMsg& event) = 0;
};

} // namespace log