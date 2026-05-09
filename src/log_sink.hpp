// log_sink.hpp
#pragma once
#include <string>

namespace log {

class Sink {
public:
    virtual ~Sink() = default;

    virtual void write(const std::string& formatted_msg) = 0;
    virtual void flush() = 0;
};

} // namespace log