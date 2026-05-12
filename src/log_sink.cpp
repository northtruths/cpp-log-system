// log_sink.cpp
#include "log_sink.hpp"
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace log
{
    // 落地控制台
    class ConsoleSink : public Sink
    {
    public:
        void write(const std::string &formatted_msg) override
        {
            std::cout << formatted_msg << std::endl;
        }

        void flush() override
        {
            std::cout << std::flush;
        }

        ~ConsoleSink()
        {
            flush();
        }
    };

    std::unique_ptr<Sink> make_console_sink()
    {
        return std::make_unique<ConsoleSink>();
    }

    // 落地文件
    class FileSink : public Sink
    {
    public:
        FileSink(const std::string &dir, const std::string &filename)
            : dir_(dir), filename_(filename)
        {
            open();
        }

        void write(const std::string &formatted_msg) override
        {
            if (file_.is_open())
            {
                file_ << formatted_msg;
            }
        }

        void flush() override
        {
            if (file_.is_open())
            {
                file_ << std::flush;
            }
        }

    private:
        void open()
        {
            mkdir(dir_.c_str(), 0755);

            std::string path = dir_ + "/" + filename_;
            file_.open(path, std::ios::app);
        }

        std::string dir_;
        std::string filename_;
        std::ofstream file_;
    };

    std::unique_ptr<Sink> make_file_sink(const std::string &dir, const std::string &filename)
    {
        return std::make_unique<FileSink>(dir, filename);
    }

    std::unique_ptr<Sink> make_file_sink(const char *dir, const char *filename)
    {
        return make_file_sink(std::string(dir), std::string(filename));
    }

    
    // 滚动文件落地
    class RollFileSink : public Sink
    {
    public:
        RollFileSink(const std::string &dir,
                     const std::string &base_name,
                     size_t max_size)
            : dir_(dir), base_name_(base_name), max_size_(max_size)
        {
            mkdir(dir_.c_str(), 0755);
            roll();
        }

        void write(const std::string &formatted_msg) override
        {
            if (!file_.is_open())
                return;

            if (cur_written_ + formatted_msg.size() > max_size_)
            {
                roll();
            }

            file_ << formatted_msg;
            cur_written_ += formatted_msg.size();
        }

        void flush() override
        {
            if (file_.is_open())
                file_ << std::flush;
        }

    private:
        void roll()
        {
            // 关闭旧文件
            if (file_.is_open())
            {
                flush();
                file_.close();
            }

            // 生成新文件名：base_name_ + 时间戳 + 序号
            std::string new_path = make_path();
            file_.open(new_path, std::ios::app);
            cur_written_ = 0;
        }

        std::string make_path()
        {
            auto t = std::time(nullptr);
            std::tm tm_buf;
            localtime_r(&t, &tm_buf);

            std::ostringstream oss;
            oss << dir_ << "/"
                << base_name_ << "_"
                << std::put_time(&tm_buf, "%Y%m%d_%H%M%S")
                << "_" << index_++
                << ".log";
            return oss.str();
        }

        std::string dir_;
        std::string base_name_;
        size_t max_size_;
        std::ofstream file_;
        size_t cur_written_ = 0;
        int index_ = 0;
    };

    std::unique_ptr<Sink> make_roll_file_sink(
        const std::string &dir,
        const std::string &base_name,
        size_t max_size)
    {
        return std::make_unique<RollFileSink>(dir, base_name, max_size);
    }

} // namespace log