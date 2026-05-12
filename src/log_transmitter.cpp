#include "log_transmitter.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace log
{
    // 同步发送
    class SyncTransmitter : public Transmitter
    {
    public:
        void send(const std::string &formatted_msg,
                  std::vector<std::unique_ptr<Sink>> &sinks) override
        {
            for (auto &sink : sinks)
            {
                sink->write(formatted_msg);
            }
        }
    };

    // 异步发送
    class AsyncTransmitter : public Transmitter
    {
    public:
        AsyncTransmitter(size_t buffer_size)
            : write_buff_(&buff_a_), flush_buff_(&buff_b_), buff_size_(buffer_size), is_running(true), is_flush_(false)
        {
            buff_a_.reserve(buff_size_);
            buff_b_.reserve(buff_size_);
            tflush_ = std::thread(&AsyncTransmitter::flush_loop, this);
        }
        ~AsyncTransmitter()
        {
            is_running.store(false, std::memory_order_release);
            {
                std::unique_lock<std::mutex> lock(mtx_buff_);
                simple_flush(lock);
            }
            while (!is_flush_.load() && tflush_.joinable())
            {
                tflush_.join();
            }
        }

        void send(const std::string &formatted_msg,
                  std::vector<std::unique_ptr<Sink>> &sinks) override
        {
            psinks_ = &sinks;
            std::unique_lock<std::mutex> lock(mtx_buff_);
            if (formatted_msg.size() + write_buff_->size() > buff_size_)
            {
                simple_flush(lock);
            }
            *write_buff_ += formatted_msg;
        }

    private:
        void swap_and_notify()
        {
            std::swap(write_buff_, flush_buff_);
            // 通知后台线程
            is_flush_.store(true, std::memory_order_release);
            cond_file_.notify_one();
        }

        void simple_flush(std::unique_lock<std::mutex> &lock)
        {
            // 如果后台线程还没处理完上一个缓冲，会阻塞业务线程
            cond_buff_.wait(lock, [this]
                            { return !is_flush_.load(); });
            swap_and_notify();
        }

        void flush_loop()
        {
            do
            {
                std::unique_lock<std::mutex> lock(mtx_file_);
                cond_file_.wait(lock, [this]()
                                { return is_flush_.load(); });
                for (auto &sink : *psinks_)
                {
                    sink->write(*flush_buff_);
                }
                flush_buff_->clear();
                is_flush_ = false;
                cond_buff_.notify_all();
            } while (is_running.load() || is_flush_.load());
        }
        // 缓冲区
        std::string buff_a_;
        std::string buff_b_;
        std::string *write_buff_;
        std::string *flush_buff_;
        size_t buff_size_;
        // 后台线程
        std::thread tflush_;
        std::mutex mtx_buff_;
        static std::mutex mtx_file_;
        std::condition_variable cond_buff_;
        std::condition_variable cond_file_;
        std::atomic<bool> is_running;
        std::atomic<bool> is_flush_;
        // 发送信息
        const std::vector<std::unique_ptr<Sink>> *psinks_;
    };
    std::mutex AsyncTransmitter::mtx_file_;

    std::unique_ptr<Transmitter> make_sync_transmitter()
    {
        return std::make_unique<SyncTransmitter>();
    }

    std::unique_ptr<Transmitter> make_async_transmitter(size_t buffer_size)
    {
        return std::make_unique<AsyncTransmitter>(buffer_size);
    }

}