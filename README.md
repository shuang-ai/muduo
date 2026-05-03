# my_muduo

一个基于 C++11 实现的轻量级网络库，参考 Muduo 设计，用于深入理解：

- Reactor 事件驱动模型
- 多线程 + IO 复用（epoll）
- 高性能 TCP 网络编程

该项目重点在于从零实现 Reactor 核心机制，而不是简单封装 socket API。

## ✨ 项目特性

- Reactor 模型（事件驱动）
- epoll + 非阻塞 IO
- one loop per thread（多线程事件循环）
- TcpServer / TcpConnection 封装
- 跨线程任务调度机制
- 轻量级日志系统

## 📁 项目结构

```
my_muduo/
├── include/mymuduo/          # 对外接口（API）
├── src/                      # 核心实现
├── example/                  # 示例（EchoServer）
├── lib/                      # 生成库
├── build/                    # 构建目录
├── autobuild.sh              # 一键编译
├── CMakeLists.txt
└── README.md
```

## ⚙️ 编译与安装

### 自动安装（推荐）

```bash
chmod +x autobuild.sh
./autobuild.sh
```

安装后：

- `/usr/include/mymuduo/`
- `/usr/lib/libmy_muduo.so`

### 手动编译

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## ▶️ 快速开始

```bash
cd example
make
./echo_server
```

测试：

```bash
telnet localhost 8888
```

## 🧠 架构设计（核心）

项目整体分为三层：

### 1️⃣ Reactor 层（事件驱动核心）

**组件：**
- EventLoop
- Channel
- Poller(epoll)
- EventLoopThreadPool

**职责：**
- IO 多路复用（epoll）
- 事件分发
- 回调执行
- 跨线程任务调度

### 2️⃣ IO 层（网络抽象）

**组件：**
- Acceptor
- TcpConnection
- Buffer
- InetAddress

**职责：**
- 封装 socket 生命周期
- 管理读写缓冲
- 提供统一 IO 接口

### 3️⃣ 业务层（用户逻辑）

**组件：**
- TcpServer
- 用户回调函数

**职责：**
- 处理业务逻辑
- 响应客户端请求

## 🔁 事件流转流程

```
客户端连接
    ↓
Acceptor（accept）
    ↓
TcpConnection 创建
    ↓
注册 Channel 到 EventLoop
    ↓
epoll 等待事件
    ↓
EventLoop 分发事件
    ↓
Channel 回调
    ↓
TcpConnection 处理
    ↓
业务回调执行
```

## ⚙️ 关键设计说明（重点🔥）

### ❓ 为什么需要 Channel？

在 epoll 模型中：

- epoll 只返回 fd + 事件
- 但程序需要：
  - 区分读/写/关闭事件
  - 调用不同的处理逻辑

👉 **Channel 的作用就是：**

将 "fd + 事件" 封装为 "事件 + 回调函数"

也就是说：
```
epoll → fd + event
        ↓
Channel → 回调函数
```

**带来的好处：**
- 解耦 epoll 与业务逻辑
- 支持不同事件绑定不同处理函数
- 提升代码可维护性

### ❓ 为什么跨线程必须唤醒 EventLoop？

**问题背景：**
- EventLoop 线程通常阻塞在 epoll_wait
- 其他线程通过 queueInLoop 投递任务

👉 **如果不唤醒：**
- EventLoop 可能一直阻塞
- 新任务无法及时执行 ❌

**解决方案：**

使用 eventfd 唤醒机制：
```
其他线程 → queueInLoop
           ↓
        写 eventfd
           ↓
epoll 返回
           ↓
EventLoop 被唤醒
           ↓
执行 doPendingFunctors
```

**核心本质：**

让"任务"也变成一种"事件"

这样可以复用 Reactor 模型统一处理。

## 🧩 核心机制总结

- 一个线程一个 EventLoop
- IO 事件驱动
- 回调机制解耦逻辑
- eventfd 实现线程间通知

## 📌 项目定位

该项目适用于：

- 学习 Reactor 模型
- 理解 Muduo 核心设计
- 网络编程实践
