// log_formatter.cpp
#include "log_formatter.hpp"
#include "log_message.hpp"
#include "log_level.hpp"
#include <sstream>
#include <ctime>
#include <memory>

namespace log
{

    class DefaultFormatter : public Formatter
    {
    public:
        std::string format(const LogMsg &msg) override
        {
            std::ostringstream oss;

            // [级别]
            oss << "[" << to_string(msg.level_) << "] ";

            // [时间]
            oss << "[" << format_time(msg.timestamp_) << "] ";

            // [文件名]
            oss << "[" << msg.file_ << "] ";

            // [行号]
            oss << "[" << msg.line_ << "] ";

            // [线程号]
            oss << "[pid " << msg.tid_ << "] ";

            // 正文
            oss << " - " << msg.content_;

            oss << "\n";
            return oss.str();
        }

    private:
        std::string format_time(const std::chrono::system_clock::time_point &tp)
        {
            auto t = std::chrono::system_clock::to_time_t(tp);
            std::tm tm_buf;
            localtime_r(&t, &tm_buf);

            char buf[32];
            //每次都要printf，可以优化
            std::snprintf(buf, sizeof(buf),
                          "%04d-%02d-%02d %02d:%02d:%02d",
                          tm_buf.tm_year + 1900,
                          tm_buf.tm_mon + 1,
                          tm_buf.tm_mday,
                          tm_buf.tm_hour,
                          tm_buf.tm_min,
                          tm_buf.tm_sec);
            return buf;
        }
    };

    // 工厂函数
    std::unique_ptr<Formatter> make_default_formatter()
    {
        return std::make_unique<DefaultFormatter>();
    }

} // namespace log