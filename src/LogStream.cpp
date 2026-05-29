#include "LogStream.hpp"
#include "logger.hpp"
#include "log_message.hpp"

namespace log
{
    LogStream::LogStream(Logger& owner, Level lv, const char* file, int line)
        : owner_(owner), lv_(lv), file_(file), line_(line) {}

    LogStream::~LogStream()
    {
        if (lv_ < owner_.min_level_.load(std::memory_order_acquire))
            return;
        LogMsg msg(lv_, file_, line_, oss_.str());
        owner_.formatter_->format(msg);
        owner_.transmitter_->send(msg.formatted_msg, owner_.sinks_, lv_ < Level::ERROR);
    }
}