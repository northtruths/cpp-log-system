// log_transmitter.hpp
#pragma once
#include "log_sink.hpp"
#include <vector>
#include <memory>
#include <string>

namespace log
{
    class Transmitter
    {
    public:
        virtual ~Transmitter() = default;

        virtual void send(const std::string& formatted_msg,
                          std::vector<std::unique_ptr<Sink>> &sinks) = 0;
    };

    std::unique_ptr<Transmitter> make_sync_transmitter();
    std::unique_ptr<Transmitter> make_async_transmitter();

} // namespace log