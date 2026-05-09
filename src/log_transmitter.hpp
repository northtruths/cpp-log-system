// log_transmitter.hpp
#pragma once
#include "log_sink.hpp"
#include "log_message.hpp"
#include "log_formatter.hpp"
#include <vector>
#include <memory>
#include <string>

namespace log
{

    // ==================== 抽象基类 ====================
    class Transmitter
    {
    public:
        virtual ~Transmitter() = default;

        // 发送日志事件：formatter 格式化 → 交给 sink 落地
        virtual void send(std::string msg,
                          std::vector<std::unique_ptr<Sink>> &sinks) = 0;
    };

    // ==================== 同步发送 ====================
    class SyncTransmitter : public Transmitter
    {
    public:
        void send(std::string msg,
                  std::vector<std::unique_ptr<Sink>> &sinks) override
        {
            for (auto &sink : sinks)
            {
                sink->write(msg);
            }
        }
    };

    // ==================== 异步发送 ====================
    class AsyncTransmitter : public Transmitter
    {
    public:
        AsyncTransmitter();
        ~AsyncTransmitter();

        void send(std::string msg,
                  std::vector<std::unique_ptr<Sink>> &sinks) override;

    private:
        // 内部缓冲区 + 后台线程
        // TODO: 具体实现（队列、双缓冲、线程等）
    };

} // namespace log