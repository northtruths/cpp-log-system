// log_sink.hpp
#pragma once
#include <string>
#include <memory>

namespace log
{

    class Sink
    {
    public:
        virtual ~Sink() = default;

        virtual void write(const std::string &formatted_msg) = 0;
        virtual void flush() = 0;
    };

    // 落地在控制台
    std::unique_ptr<Sink> make_console_sink();

    // 落地在文件
    std::unique_ptr<Sink> make_file_sink();
    std::unique_ptr<Sink> make_file_sink(const std::string &dir, const std::string &filename);
    std::unique_ptr<Sink> make_file_sink(const char *dir, const char *filename);
} // namespace log