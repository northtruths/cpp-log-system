// tests/speed_test.cpp
#include "logger.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>

int main()
{
    auto& logger = mylog::Logger::instance();

    logger.set_level(mylog::Level::TRACE);
    logger.set_formatter(mylog::make_default_formatter());
    logger.set_transmitter(mylog::make_async_transmitter(4 * 1024 * 1024, 1000));
    logger.add_sink(mylog::make_file_sink("./log", "perf.log"));

    const int total = 1000000;
    const int thread_count = 4;

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int t = 0; t < thread_count; ++t)
    {
        threads.emplace_back([&logger, t, total, thread_count]()
        {
            int per_thread = total / thread_count;
            for (int i = 0; i < per_thread; ++i)
            {
                std::ostringstream oss;
                oss << "线程" << t << "消息" << i;
                LOG_INFO(oss.str().c_str());
            }
        });
    }

    for (auto& th : threads)
        th.join();

    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "总条数: " << total << std::endl;
    std::cout << "线程数: " << thread_count << std::endl;
    std::cout << "耗时: " << ms << " 毫秒" << std::endl;
    if (ms > 0)
        std::cout << "吞吐量: " << total * 1000 / ms << " 条/秒" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}