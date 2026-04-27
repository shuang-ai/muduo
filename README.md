# my_muduo

基于 C++17 实现的轻量级网络库，参考 muduo 设计，用于学习 Reactor 模型和网络编程。

## 项目特性

- 基于 Reactor 模型的事件驱动架构
- 非阻塞 IO + epoll (Linux)
- 多线程 + 事件循环
- 支持 TcpServer / TcpConnection 封装
- 日志系统
- 适用于 Linux 平台

## 项目结构

```text
my_muduo/
├── include/mymuduo/          # 头文件目录（用户可见）
│   ├── Acceptor.h
│   ├── Buffer.h
│   ├── Channel.h
│   ├── EventLoop.h
│   ├── EventLoopThreadPool.h
│   ├── InetAddress.h
│   ├── Logger.h
│   ├── TcpConnection.h
│   ├── TcpServer.h
│   └── ...
├── src/                      # 源文件目录
│   ├── Acceptor.cc
│   ├── Buffer.cc
│   ├── Channel.cc
│   ├── EventLoop.cc
│   ├── TcpConnection.cc
│   ├── TcpServer.cc
│   └── ...
├── example/                  # 示例程序
│   ├── echo_server.cc
│   └── Makefile
├── lib/                      # 编译生成的库文件
├── build/                    # 编译临时目录
├── autobuild.sh              # 一键编译安装脚本
├── CMakeLists.txt            # CMake 配置
└── README.md                 # 项目说明

编译安装
方式一：使用 autobuild.sh（推荐）
给脚本执行权限：

bash
chmod +x autobuild.sh
编译并安装到系统：

bash
./autobuild.sh
安装后：

头文件位置：/usr/include/mymuduo/

库文件位置：/usr/lib/libmy_muduo.so

方式二：手动 CMake 编译
bash
mkdir build && cd build
cmake ..
make -j$(nproc)
cd ..
编译后库文件位于 lib/libmy_muduo.so，头文件位于 include/mymuduo/。

运行示例
编译示例程序
bash
cd example
make
启动 EchoServer
bash
./echo_server
输出示例：

text
[INFO] 2026/04/27 19:32:43 : EventLoop 0x7f... start looping 
[INFO] 2026/04/27 19:32:43 : EchoServer is running, listening on 8888
测试连接
使用 telnet 或 nc 工具测试：

bash
# 方式一：telnet
telnet localhost 8888

# 方式二：nc (netcat)
echo "Hello" | nc localhost 8888
