一般正式项目的目录结构
cpp-log-system/
│
├── README.md
├── .gitignore
├── Makefile
│
├── logs/
│
├── build/
│
├── tests/
│
├── example/
│
├── src/
│   ├── util.hpp
│   ├── level.hpp
│   ├── message.hpp
│   ├── formatter.hpp
│   ├── sink.hpp
│   ├── logger.hpp
│   ├── looper.hpp
│   └── manager.hpp
│
└── bin/

## 已完成
项目模块化设计
目前框架已经基本实现 

## 未完成：
滚动模块
扩展模块
各逻辑刷新：高危等级日志立即刷新、
滚动模块时间滚动：通过tm来获取这次和上次的具体时间，再根据逻辑复用roll就行了（目前不想写了）

程序正常退出（exit() / return）	依赖对象析构，需要你的 Logger 是全局或 static	✅ 是
信号处理（SIGINT / SIGTERM）	Ctrl+C 不会调用析构	⚠️ 强烈建议
日志等级切换	新等级生效前把当前缓冲区刷掉	看需求
手动轮转日志文件	切文件前确保旧文件数据完整	✅ 是（文件滚动）
异常捕获中（catch）	异常可能导致提前退出	⚠️ 可选
单元测试/基准测试

## 可优化：
格式化模块，格式化时间每次都要snprintf，可以优化
全局控制模块，正文信息支持 << 流样式，目前只支持string输入

## 额外学习知识
**接口兼容**：对外接口用c，内部实现无所谓，这样其他语言可以用通用的c接口来使用我的项目
例：一个函数，接受参数c字符串的话，其他语言都可以传，我内部用c++用其他的语言处理都行，只要把库链接好，但如果参数只接受std::string的话，其他语言哪来的std::string给你

