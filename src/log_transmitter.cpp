#include "log_transmitter.hpp"
#include<thread>
#include<mutex>
#include<condition_variable>

namespace log
{
    // 同步发送
    class SyncTransmitter : public Transmitter
    {
    public:
        void send(const std::string& formatted_msg,
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
        AsyncTransmitter()
            : write_buff_(&buff_a_), flush_buff_(&buff_b_), is_flush_(false)
        {
            buff_a_.reserve(buff_size_);
            buff_b_.reserve(buff_size_);
        }
        ~AsyncTransmitter();

        void send(const std::string &formatted_msg,
                  std::vector<std::unique_ptr<Sink>> &sinks) override
        {
            psinks_ = &sinks;
            std::unique_lock<std::mutex> lock(mtx_buff_);
            if (formatted_msg.size() + write_buff_->size() > buff_size_)
            {
                cond_buff_.wait(lock, [this](){return !is_flush_;});
                std::swap(write_buff_, flush_buff_);
                //通知后台线程
                is_flush_ = true;
                cond_file_.notify_one();
            }
            *write_buff_ += formatted_msg;
        }

    private:
        void flush(){
            std::unique_lock<std::mutex> lock(mtx_file_);
            cond_file_.wait(lock, [this](){return is_flush_;});
            for (auto &sink : *psinks_)
            {
                sink->write(*flush_buff_);
            }
            is_flush_ = false;
            cond_buff_.notify_all();
        }
        //缓冲区
        std::string buff_a_;
        std::string buff_b_;
        std::string *write_buff_;
        std::string *flush_buff_;
        size_t buff_size_;
        //后台线程
        std::thread tflush_;
        std::mutex mtx_buff_;
        static std::mutex mtx_file_;
        std::condition_variable cond_buff_;
        std::condition_variable cond_file_;
        bool is_flush_;
        //发送信息
        const std::vector<std::unique_ptr<Sink>>* psinks_;
    };

    std::unique_ptr<Transmitter> make_sync_transmitter()
    {
        return std::make_unique<SyncTransmitter>();
    }

    std::unique_ptr<Transmitter> make_async_transmitter()
    {
        return std::make_unique<AsyncTransmitter>();
    }

}