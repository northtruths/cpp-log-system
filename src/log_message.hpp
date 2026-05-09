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
    std::thread::id                     tid_;
    int                                 line_;
    std::string                         content_;

    LogMsg(Level lv, const char* f, int l, std::string msg)
        : level_(lv)
        , timestamp_(std::chrono::system_clock::now())
        , file_(f)
        , tid_(std::this_thread::get_id())
        , line_(l)
        , content_(std::move(msg))
    {}
};

} // namespace log