// log_sink.cpp
#include "log_sink.hpp"
#include <iostream>
#include <memory>
#include <sys/stat.h>
#include <fstream>

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
        FileSink()
            : dir_("./log"), filename_("log.txt")
        {
            open();
        }

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

    std::unique_ptr<Sink> make_file_sink()
    {
        return std::make_unique<FileSink>();
    }

    std::unique_ptr<Sink> make_file_sink(const std::string &dir, const std::string &filename)
    {
        return std::make_unique<FileSink>(dir, filename);
    }

    std::unique_ptr<Sink> make_file_sink(const char *dir, const char *filename)
    {
        return make_file_sink(std::string(dir), std::string(filename));
    }
} // namespace log