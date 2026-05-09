// logger.hpp
#pragma once
#include "log_level.hpp"
#include "log_message.hpp"
#include "log_formatter.hpp"
#include "log_sink.hpp"
#include "log_transmitter.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace log
{
    class Logger
    {
    public:
        static Logger &instance()
        {
            static Logger logger;
            return logger;
        }

        void set_level(const std::string &level)
        {
            min_level_.store(from_string(level), std::memory_order_release);
        }

        void set_formatter(std::unique_ptr<Formatter> fmt)
        {
            formatter_ = std::move(fmt);
        }

        void set_transmitter(std::unique_ptr<Transmitter> transmitter)
        {
            transmitter_ = std::move(transmitter);
        }

        void add_sink(std::unique_ptr<Sink> sink)
        {
            sinks_.push_back(std::move(sink));
        }

        void clear_sinks()
        {
            sinks_.clear();
        }

        void operator()(const Level &lv, const char *f, int l, const std::string &content)
        {
            if (lv < min_level_.load(std::memory_order_acquire))
                return;
            LogMsg event(lv, f, l, content);
            std::string msg = formatter_->format(event);
            transmitter_->send(msg, sinks_);
        }

    private:
        std::atomic<Level> min_level_;
        std::unique_ptr<Formatter> formatter_;
        std::vector<std::unique_ptr<Sink>> sinks_;
        std::unique_ptr<Transmitter> transmitter_;
    };

#define LOG_TRACE(logger, content) logger(log::Level::TRACE, __FILE__, __LINE__, content)
#define LOG_DEBUG(logger, content) logger(log::Level::DEBUG, __FILE__, __LINE__, content)
#define LOG_INFO(logger, content) logger(log::Level::INFO, __FILE__, __LINE__, content)
#define LOG_WARN(logger, content) logger(log::Level::WARN, __FILE__, __LINE__, content)
#define LOG_ERROR(logger, content) logger(log::Level::ERROR, __FILE__, __LINE__, content)
#define LOG_FATAL(logger, content) logger(log::Level::FATAL, __FILE__, __LINE__, content)
} // namespace log
