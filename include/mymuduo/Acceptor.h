#pragma once
#include "noncopyable.h"
#include <functional>
// #include "InetAddress.h"
// #include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

// 这个前置声明可以防止头文件重复依赖？
// 因为这个类需要获取这两个指针或者引用，指针是固定大小类型，编译器不报错
class EventLoop;
class InetAddress;

class Acceptor : noncopyable{
public:
  // sockfd通信套接字
  // 客户端的地址
  using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
  // reuseport 控制是否设置 SO_REUSEPORT 套接字选项。
  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

  ~Acceptor();

      void setNewConnectionCallback(const NewConnectionCallback &cb) 
    {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();
    
    // Acceptor 运行在用户指定的 EventLoop 上。
// 在 TcpServer 中，这个 EventLoop 就是主 Reactor（mainLoop），
// 专门负责监听和接受新连接。
    EventLoop *loop_; 
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;

};
