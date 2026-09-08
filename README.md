# cpp-log-system 使用手册

一个轻量、易扩展的 C++11 日志组件,支持多级别日志、同步/异步发送、多落地目标(控制台 / 文件 / 滚动文件)、多线程并发写入。

- 命名空间:`mylog`
- 编译器要求:g++ 支持 C++11
- 依赖:pthread

---

## 一、目录结构

```
cpp-log-system/
├── Makefile              构建脚本
├── include/             公共头文件(对外 API)
│   ├── log_level.hpp        级别枚举 + 字符串/枚举互转
│   ├── log_message.hpp      日志信息结构体
│   ├── log_stream.hpp       流式日志(支持 << 拼接)
│   ├── log_formatter.hpp   格式化接口 + 工厂
│   ├── log_sink.hpp         落地接口 + 三个内置 Sink + 工厂
│   ├── log_transmitter.hpp  发送接口 + 同步/异步工厂
│   └── logger.hpp           全局 Logger + 日志宏
├── src/                 实现
│   ├── log_formatter.cpp
│   ├── log_sink.cpp
│   ├── log_stream.cpp
│   └── log_transmitter.cpp
├── docs/                设计与说明
├── tests/               测试目录(main.cpp 入口)
└── bin/                 可执行输出
```

---

## 二、快速开始

### 2.1 编译

```bash
make          # 编译,产出 bin/test
make run       # 编译并运行
make clean     # 清理 build/ bin/ logs/ log/
```

编译选项(见 [Makefile](../Makefile)):
- 标准:`-std=c++11`
- 链接库:`-lpthread`
- 优化:`-O2`

### 2.2 最简使用(五分钟接入)

只需 `#include "logger.hpp"`,默认 Logger 已装配 `DefaultFormatter` 与 `AsyncTransmitter`,但**默认没有任何 sink**,需要至少添加一个落地目标。

```cpp
#include "logger.hpp"
using namespace mylog;

int main() {
    // 1. 添加落地目标(至少一个)
    logger.add_sink(make_console_sink());
    logger.add_sink(make_file_sink("./logs", "app.log"));

    // 2. 注册信号处理(可选)
    logger.register_signal_handler();

    // 3. 设置最低级别(低于该级别的日志被屏蔽)
    logger.set_level("INFO");

    // 4. 输出日志
    LOG_INFO("服务启动成功");
    LOG_ERROR("连接失败");

    return 0;
}
```

> **重要提示**:`LOG_xxx(content)` 宏的 `content` 必须是 `std::string`(或可隐式转换),**不是 printf 风格的可变参**。如需拼接数值,请用 `LOG_INFO_STREAM() << "x=" << x;`。

---

## 三、日志级别

定义在 [include/log_level.hpp](../include/log_level.hpp#L6-L15):

| 枚举 | 整数值 | 字符串 |
|------|------|------|
| TRACE | 0 | "TRACE" |
| DEBUG | 1 | "DEBUG" |
| INFO  | 2 | "INFO"  |
| WARN  | 3 | "WARN"  |
| ERROR | 4 | "ERROR" |
| FATAL | 5 | "FATAL" |

**等级数值有顺序意义**:级别越高,数字越大。`set_level("WARN")` 表示只输出 `WARN/ERROR/FATAL`,屏蔽 `TRACE/DEBUG/INFO`。

字符与枚举互转的辅助函数:
- `inline const char* level_to_string(Level lv)`
- `inline Level from_string(const std::string& s)`(未知字符串默认返回 `INFO`)

---

## 四、日志宏(API 入口)

定义在 [include/logger.hpp](../include/logger.hpp#L141-L155)。

### 4.1 字符串版

```cpp
LOG_TRACE(content)
LOG_DEBUG(content)
LOG_INFO(content)
LOG_WARN(content)
LOG_ERROR(content)
LOG_FATAL(content)
```

`content` 必须能构造 `std::string`。宏内部自动捕获 `__FILE__` 与 `__LINE__`。

示例:
```cpp
LOG_INFO("用户登录");
LOG_ERROR(std::string("errno=") + std::to_string(errno));
```

### 4.2 流式版(支持任意类型 `<<`)

```cpp
LOG_TRACE_STREAM()
LOG_DEBUG_STREAM()
LOG_INFO_STREAM()
LOG_WARN_STREAM()
LOG_ERROR_STREAM()
LOG_FATAL_STREAM()
```

内部使用 `std::ostringstream`,支持任何可 `<<` 输出的类型。

示例:
```cpp
LOG_INFO_STREAM() << "user=" << uid << " ip=" << ip << " cost=" << cost << "ms";
```

---

## 五、全局 Logger 接口

全局变量 `mylog::logger`(单例,见 [logger.hpp:139](../include/logger.hpp#L139))。

| 方法 | 说明 |
|------|------|
| `void set_level(const std::string& level)` | 设置最低日志级别("TRACE"/"DEBUG"/...) |
| `void set_danger_level(const std::string& level)` | 设置"危险级别",低于此级别的日志会触发同步强制 flush(立即落地) |
| `void set_formatter(std::unique_ptr<Formatter>)` | 设置格式化器 |
| `std::unique_ptr<Formatter>& get_formatter()` | 获取当前格式化器引用 |
| `void set_transmitter(std::unique_ptr<Transmitter>)` | 设置发送器(同步/异步) |
| `std::unique_ptr<Transmitter>& get_transmitter()` | 获取当前发送器引用 |
| `void add_sink(std::unique_ptr<Sink>)` | 追加一个落地目标 |
| `void clear_sinks()` | 清空所有 sink |
| `void register_signal_handler()` | 注册 SIGINT/SIGTERM/SIGSEGV 处理,Ctrl+C 时强制 flush |
| `bool should_exit() const` | 查询是否收到退出信号 |
| `void flush_all()` | 强制 flush 所有 sink(信号处理函数内部调用) |

### 默认装配

`Logger` 默认构造时已装配([logger.hpp:113-117](../include/logger.hpp#L113-L117)):
- 最低级别:`Level::TRACE`(输出所有)
- 格式化器:`make_default_formatter()`
- 发送器:`make_async_transmitter()`(默认 1MB 缓冲、1000ms 刷新)
- 危险级别:`Level::ERROR`(ERROR/FATAL 会触发立即 flush)
- sinks:空(必须自行 `add_sink`)

---

## 六、Sink(落地目标)

定义在 [include/log_sink.hpp](../include/log_sink.hpp)。

### 6.1 内置 Sink

| 类 | 工厂函数 | 说明 |
|----|---------|------|
| `ConsoleSink` | `make_console_sink()` | 输出到 `std::cout`,析构时 flush |
| `FileSink` | `make_file_sink(dir, filename)` | 追加写入 `dir/filename`,不存在目录自动 `mkdir` |
| `RollFileSink` | `make_roll_file_sink(dir, base_name, max_size)` | 滚动文件,单文件超过 `max_size` 时切换新文件,并按天分目录 |

`make_file_sink` 重载:
```cpp
std::unique_ptr<Sink> make_file_sink(const std::string& dir = "./logs",
                                     const std::string& filename = "app.log");
std::unique_ptr<Sink> make_file_sink(const char* dir, const char* filename);
```

`make_roll_file_sink`:
```cpp
std::unique_ptr<Sink> make_roll_file_sink(
    const std::string& dir = "./logs",
    const std::string& base_name = "app.log",
    size_t max_size = 10 * 1024 * 1024);  // 默认 10MB
```

### 6.2 滚动文件命名规则

见 [src/log_sink.cpp:126-138](../src/log_sink.cpp#L126-L138):
- 按天分目录:`<dir>/<YYYY>_<M>_<D>/`
- 文件名:`<base_name>_<YYYYMMDD>_<HHMMSS>_<index>.log`
- 当 `cur_written_ + formatted_msg.size() > max_size_` 时调用 `roll()` 切换

### 6.3 自定义 Sink

继承 `Sink`,实现两个纯虚函数:
```cpp
class MySink : public mylog::Sink {
public:
    void write(const std::string& formatted_msg) override {
        // 你的落地逻辑:网络发送、DB 写入...
    }
    void flush() override {
        // 刷盘
    }
};
```

使用:
```cpp
logger.add_sink(std::make_unique<MySink>());
```

---

## 七、Formatter(格式化器)

定义在 [include/log_formatter.hpp](../include/log_formatter.hpp)。

### 7.1 默认格式化器

`make_default_formatter()` 返回 `DefaultFormatter`,输出格式:

```
[<级别>] [<时间 YYYY-MM-DD HH:MM:SS>] [ <文件名> ] [<行号>] [pid <线程哈希>]  - <正文>
```

示例输出:
```
[INFO] [2026-09-08 14:30:21] [ ./main.cpp ] [12] [pid 3829503]  - 服务启动成功
```

### 7.2 自定义格式化器

继承 `Formatter`,实现:
```cpp
class MyFormatter : public mylog::Formatter {
public:
    std::string format(LogMsg& msg) override {
        std::ostringstream oss;
        oss << "[" << level_to_string(msg.level_) << "] "
            << msg.content_;
        return oss.str();
    }
};
```

替换:
```cpp
logger.set_formatter(std::make_unique<MyFormatter>());
```

### 7.3 LogMsg 字段

`LogMsg` 结构([include/log_message.hpp](../include/log_message.hpp#L10-L26)):

| 字段 | 类型 | 说明 |
|------|------|------|
| `level_` | `Level` | 日志级别 |
| `timestamp_` | `std::chrono::system_clock::time_point` | 时间点 |
| `file_` | `const char*` | 源文件名(`__FILE__`) |
| `line_` | `int` | 行号 |
| `tid_` | `std::thread::id` | 线程 ID |
| `content_` | `std::string` | 正文 |

---

## 八、Transmitter(发送器)

定义在 [include/log_transmitter.hpp](../include/log_transmitter.hpp)。

| 发送器 | 工厂函数 | 特点 |
|-------|---------|------|
| `SyncTransmitter` | `make_sync_transmitter()` | 调用线程直接调用 sink,简单但慢 |
| `AsyncTransmitter` | `make_async_transmitter(buffer_size, flush_interval_ms)` | 双缓冲 + 后台线程,高性能 |

### 8.1 同步 vs 异步

**同步**:
- 每条日志调用线程同步完成格式化、写入、flush
- 简单可靠,适合调试或低 QPS 场景
- 默认危险级别以上的日志使用同步路径(立即 flush)

**异步**(默认):
- 写入线程把消息追加到 `write_buff_`,达到阈值触发 swap
- 后台 `flush_loop` 线程消费 `flush_buff_`,周期由 `flush_interval_ms` 控制
- 双缓冲 `std::swap` 交换指针,减小锁竞争
- 缓冲满时丢弃新日志(可通过 `is_safe` 参数让危险日志立即 flush)

### 8.2 切换发送器

```cpp
// 同步
logger.set_transmitter(make_sync_transmitter());

// 异步,5MB 缓冲,200ms 刷新
logger.set_transmitter(make_async_transmitter(5 * 1024 * 1024, 200));
```

### 8.3 危险级别的工作机制

`Logger::operator()` 会传 `lv < danger_level_` 给 `Transmitter::send(..., is_safe)`:
- `is_safe == true`:该日志是"安全级别"(如 TRACE/DEBUG),走缓冲路径,可能延迟落地
- `is_safe == false`:该日志是"危险级别"(如 ERROR/FATAL),立即调用 `flush()` 强制刷盘

通过 `set_danger_level("WARN")` 可让 WARN 及以上日志都走同步强制 flush 路径。

### 8.4 自定义发送器

继承 `Transmitter`,实现:
```cpp
class MyTransmitter : public mylog::Transmitter {
public:
    void send(const std::string& formatted_msg,
              std::vector<std::unique_ptr<Sink>>& sinks,
              bool is_safe) override {
        // 自定义分发逻辑
    }
};
```

---

## 九、信号处理与优雅退出

见 [include/logger.hpp:24-33](../include/logger.hpp#L24-L33)。

```cpp
logger.register_signal_handler();
```

注册了 SIGINT/SIGTERM/SIGSEGV 处理:
1. 触发时调用 `instance().flush_all()` 强制刷新所有 sink
2. 恢复信号默认行为
3. `raise(sig)` 让程序正常退出

> 这解决了异步发送模式下 Ctrl+C 直接终止导致日志丢失的问题。强烈推荐在 `main` 入口处调用一次。

---

## 十、完整示例

### 10.1 控制台 + 文件 + 滚动文件

```cpp
#include "logger.hpp"
using namespace mylog;

int main() {
    // 信号处理
    logger.register_signal_handler();

    // 级别配置
    logger.set_level("DEBUG");
    logger.set_danger_level("ERROR");   // ERROR/FATAL 立即刷盘

    // 落地目标
    logger.add_sink(make_console_sink());
    logger.add_sink(make_file_sink("./logs", "app.log"));
    logger.add_sink(make_roll_file_sink("./logs", "roll", 10 * 1024 * 1024));  // 10MB 滚动

    // 异步发送,2MB 缓冲,500ms 刷新
    logger.set_transmitter(make_async_transmitter(2 * 1024 * 1024, 500));

    // 字符串版
    LOG_DEBUG("调试信息");
    LOG_INFO("服务启动");
    LOG_WARN("警告");
    LOG_ERROR("错误发生");

    // 流式版(支持拼接任意类型)
    int uid = 1001;
    double cost = 3.14;
    LOG_INFO_STREAM() << "uid=" << uid << " cost=" << cost << "ms";

    return 0;
}
```

### 10.2 自定义 Sink + Formatter

```cpp
#include "logger.hpp"
using namespace mylog;

// 自定义 sink:写到网络(占位)
class NetworkSink : public Sink {
public:
    void write(const std::string& formatted_msg) override {
        // send(socket_fd, formatted_msg.data(), formatted_msg.size(), 0);
    }
    void flush() override { /* ... */ }
};

// 自定义 formatter:JSON
class JsonFormatter : public Formatter {
public:
    std::string format(LogMsg& msg) override {
        std::string s = "{\"level\":\"";
        s += level_to_string(msg.level_);
        s += "\",\"file\":\"";
        s += msg.file_;
        s += "\",\"line\":";
        s += std::to_string(msg.line_);
        s += ",\"msg\":\"";
        s += msg.content_;
        s += "\"}";
        return s;
    }
};

int main() {
    logger.add_sink(std::make_unique<NetworkSink>());
    logger.set_formatter(std::make_unique<JsonFormatter>());
    LOG_INFO("hello");
    return 0;
}
```

---

## 十一、多线程使用

`Logger` 是单例,内部各模块已做线程安全:
- `sinks_` 的写操作未加锁,**应在多线程启动前完成 `add_sink` 配置,运行时不要动态增删**
- 异步发送模式下,`send()` 用 `std::lock_guard<std::mutex>` 保护 `write_buff_`
- 后台 `flush_loop` 持有独立的 `mtx_buff_` / `mtx_file_`,与写入线程解耦

典型用法:
```cpp
logger.add_sink(make_file_sink("./logs", "app.log"));
logger.set_level("INFO");

std::vector<std::thread> ts;
for (int i = 0; i < 4; ++i) {
    ts.emplace_back([i] {
        for (int j = 0; j < 100000; ++j)
            LOG_INFO_STREAM() << "thread=" << i << " idx=" << j;
    });
}
for (auto& t : ts) t.join();
```

参考性能([docs/study_log.md](./study_log.md#L94-L108)):4 线程 100 万条,O2 优化下约 94 万条/秒。

---

## 十二、常见问题

**Q1: 为什么没有任何输出?**
A: 默认 sinks 为空。必须至少 `logger.add_sink(make_console_sink());` 或文件 sink。

**Q2: Ctrl+C 退出后日志丢失?**
A: 异步模式下,后台缓冲未来得及刷盘。解决:`logger.register_signal_handler();`(在 `main` 开头调用一次)。

**Q3: 数值/浮点怎么拼到日志里?**
A: 字符串版 `LOG_INFO(content)` 不支持 printf 风格,需用流式版:
```cpp
LOG_INFO_STREAM() << "x=" << x << ", y=" << y;
```

**Q4: 想让 ERROR 及以上立即落地,怎么做?**
A: `logger.set_danger_level("ERROR");`。低于该级别的日志会触发 `is_safe=false` 路径,立即 flush。

**Q5: 如何切换为同步模式(调试时定位)?**
A: `logger.set_transmitter(make_sync_transmitter());`,日志立即同步落地,方便调试。

**Q6: 编译报错找不到 `logger.hpp`?**
A: 编译命令需加 `-Iinclude`,例如 `g++ -std=c++11 -Iinclude -pthread main.cpp src/*.cpp -o app`。

---

## 十三、API 速查

```cpp
namespace mylog {
    // 级别
    enum class Level : int { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LEVELS };
    inline const char* level_to_string(Level);
    inline Level from_string(const std::string&);

    // 全局单例
    static Logger& logger = Logger::instance();

    // 宏
    #define LOG_TRACE(content) ...
    #define LOG_DEBUG(content) ...
    #define LOG_INFO(content)  ...
    #define LOG_WARN(content)  ...
    #define LOG_ERROR(content) ...
    #define LOG_FATAL(content) ...

    #define LOG_TRACE_STREAM() ...
    #define LOG_DEBUG_STREAM() ...
    #define LOG_INFO_STREAM()  ...
    #define LOG_WARN_STREAM()  ...
    #define LOG_ERROR_STREAM() ...
    #define LOG_FATAL_STREAM() ...

    // 工厂
    std::unique_ptr<Formatter>    make_default_formatter();
    std::unique_ptr<Sink>        make_console_sink();
    std::unique_ptr<Sink>        make_file_sink(const std::string& dir = "./logs",
                                                const std::string& filename = "app.log");
    std::unique_ptr<Sink>        make_roll_file_sink(const std::string& dir = "./logs",
                                                     const std::string& base_name = "app.log",
                                                     size_t max_size = 10 * 1024 * 1024);
    std::unique_ptr<Transmitter> make_sync_transmitter();
    std::unique_ptr<Transmitter> make_async_transmitter(size_t buffer_size = 1 * 1024 * 1024,
                                                         int flush_interval_ms = 1000);
}
```

---

## 十四、扩展指南

### 14.1 新增 Sink 类型

1. 在 `include/log_sink.hpp` 增加派生类声明
2. 在 `src/log_sink.cpp` 实现 `write()` / `flush()`
3. 增加工厂函数(可选)
4. 使用:`logger.add_sink(std::make_unique<MySink>())`

### 14.2 新增 Formatter

1. 在 `include/log_formatter.hpp` 增加派生类
2. 实现 `std::string format(LogMsg& msg)`
3. `logger.set_formatter(std::make_unique<MyFormatter>())`

### 14.3 新增 Transmitter

1. 在 `include/log_transmitter.hpp` 增加派生类
2. 在 `src/log_transmitter.cpp` 实现 `send()`
3. `logger.set_transmitter(std::make_unique<MyTransmitter>())`

---

## 附:编译与链接

源文件清单(Makefile 会自动 `wildcard src/*.cpp`):
- `src/log_formatter.cpp`
- `src/log_sink.cpp`
- `src/log_stream.cpp`
- `src/log_transmitter.cpp`

最小手动编译命令:
```bash
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude \
    main.cpp src/*.cpp \
    -o app -lpthread
```
