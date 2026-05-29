// logger.hpp
#pragma once
#include "log_level.hpp"
#include "log_message.hpp"
#include "log_formatter.hpp"
#include "log_sink.hpp"
#include "log_transmitter.hpp"
#include "LogStream.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <sstream>

namespace log
{
    class Logger
    {
        friend class LogStream;
    public:
        static Logger &instance()
        {
            static Logger logger;
            return logger;
        }

        void set_level(const Level &level)
        {
            min_level_.store(level, std::memory_order_release);
        }

        void set_formatter(std::unique_ptr<Formatter> fmt)
        {
            formatter_ = std::move(fmt);
        }

        std::unique_ptr<Formatter> &get_formatter()
        {
            return formatter_;
        }

        void set_transmitter(std::unique_ptr<Transmitter> transmitter)
        {
            transmitter_ = std::move(transmitter);
        }

        std::unique_ptr<Transmitter> &get_transmitter()
        {
            return transmitter_;
        }

        void add_sink(std::unique_ptr<Sink> sink)
        {
            sinks_.push_back(std::move(sink));
        }

        void clear_sinks()
        {
            sinks_.clear();
        }

        void set_danger_level(const Level &lv)
        {
            danger_level_ = lv;
        }

        void operator()(const Level lv, const char *file, int line, const char *content)
        {
            if (lv < min_level_.load(std::memory_order_acquire))
                return;
            LogMsg msg(lv, file, line, content);
            // 格式化可以放进transmitter后台线程解决，可以减业务消耗，但这会稍微破坏框架设计
            formatter_->format(msg);
            transmitter_->send(msg.formatted_msg, sinks_, lv < Level::ERROR);
        }

        LogStream operator()(const Level lv, const char *file, int line)
        {
            return LogStream(*this, lv,  file, line);
        }

    private:
        Logger()
            : min_level_(Level::TRACE), formatter_(make_default_formatter()), transmitter_(make_async_transmitter())
        {
        }

        std::atomic<Level> min_level_;
        std::unique_ptr<Formatter> formatter_;
        std::vector<std::unique_ptr<Sink>> sinks_;
        std::unique_ptr<Transmitter> transmitter_;
        Level danger_level_ = Level::ERROR;
    };
    

//字符串版 —— 直接传内容
#define LOG_TRACE(content)   logger(log::Level::TRACE, __FILE__, __LINE__, content)
#define LOG_DEBUG(content)   logger(log::Level::DEBUG, __FILE__, __LINE__, content)
#define LOG_INFO(content)    logger(log::Level::INFO,  __FILE__, __LINE__, content)
#define LOG_WARN(content)    logger(log::Level::WARN,  __FILE__, __LINE__, content)
#define LOG_ERROR(content)   logger(log::Level::ERROR, __FILE__, __LINE__, content)
#define LOG_FATAL(content)   logger(log::Level::FATAL, __FILE__, __LINE__, content)

//流式版 —— 不传 content，用 << 追加
#define LOG_TRACE_STREAM()   logger(log::Level::TRACE, __FILE__, __LINE__)
#define LOG_DEBUG_STREAM()   logger(log::Level::DEBUG, __FILE__, __LINE__)
#define LOG_INFO_STREAM()    logger(log::Level::INFO,  __FILE__, __LINE__)
#define LOG_WARN_STREAM()    logger(log::Level::WARN,  __FILE__, __LINE__)
#define LOG_ERROR_STREAM()   logger(log::Level::ERROR, __FILE__, __LINE__)
#define LOG_FATAL_STREAM()   logger(log::Level::FATAL, __FILE__, __LINE__)
} // namespace log
