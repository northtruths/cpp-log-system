#include "logger.hpp"
#include <unistd.h>
#include <iostream>
#include <string>

int main()
{
    auto &logger = mylog::Logger::instance();
    // logger.set_level(log::Level::INFO);
    // //logger.set_transmitter(log::make_sync_transmitter());

    //sink
    logger.add_sink(mylog::make_console_sink());
        //logger.add_sink(log::make_file_sink("logs/log", "test.log"));
    logger.add_sink(mylog::make_roll_file_sink("./logs", "test", 1 * 1024*1024));
    int i = 0;
    while (true)
    {
        //LOG_DEBUG(("测试" + std::to_string(i)).c_str());
        LOG_DEBUG_STREAM() << "test1" << "test2" << "后面是数字: " << 1 << ' ' << 20.4 << " " << -11.2234;
        // std::cout << "上面log是debug级，所以不会立即刷新，这句话上面也不会出现测试" << std::endl;
        // LOG_ERROR(logger, "错误" +  std::to_string(i));
        // std::cout << "上面log是error级，所以会立即刷新，这句话之前会看见以error日志结尾的一串日志" << std::endl;
        sleep(1);
        i = (i + 1 ) % INT32_MAX;
    }
    return 0;
}