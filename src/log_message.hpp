// log_message.hpp
#pragma once
#include "log_level.hpp"
#include <string>
#include <chrono>
#include <thread>

namespace log {

struct LogMsg {
    Level                               level_;
    std::chrono::system_clock::time_point timestamp_;
    const char*                         file_;
    int                                 line_;
    std::thread::id                     tid_;
    std::string                         content_;

    LogMsg(Level lv, const char* f, int l, std::string content)
        : level_(lv)
        , timestamp_(std::chrono::system_clock::now())
        , file_(f)
        , line_(l)
        , tid_(std::this_thread::get_id())
        , content_(std::move(content))
    {}
};

} // namespace log