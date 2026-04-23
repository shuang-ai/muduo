#pragma once
#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
class EventLoop;
// class TcpConnectionPtr;
class TcpServer : noncopyable {
public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop *loop, const InetAddress &listenAddr,
            const std::string &nameArg, Option option = kNoReusePort);
  ~TcpServer();

  // 设置线程初始化回调函数
  // 当EventLoopThread创建新线程并启动EventLoop时，会先调用这个回调
  // 通常用于设置线程局部变量、线程优先级等初始化操作
  void setThreadInitcallback(const ThreadInitCallback &cb) {
    threadInitCallback_ = cb;
  }

  // 设置连接建立/断开回调函数
  // 当TCP连接建立成功（accept接收新连接）或连接断开时触发
  // 参数为TcpConnectionPtr，可用于判断连接状态、记录日志等
  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  // 设置消息接收回调函数（读回调）
  // 当socket可读，并且muduo已经将数据读取到inputBuffer后触发
  // 参数为连接对象指针和缓冲区，用户在这里处理业务逻辑（解析协议、处理请求）
  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  // 设置写完成回调函数
  // 当outputBuffer_中的所有数据都已通过write()系统调用交给内核发送缓冲区后触发
  // 注意：这不是"数据发送到对方"的确认，而是"数据已交给内核"的通知
  // 通常用于发送大文件时的流控：发完一块后再发下一块，避免用户态缓冲区无限膨胀
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }

  // 设置底层subloop的个数
  void setThreadNum(int numThreads);

  // 开启服务器监听
  void start();

private:
  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnectionInLoop(const TcpConnectionPtr &conn);

  // 因为 fd 会被内核快速重用，
  // 用 string 做 key 能避免错把新连接当成旧连接来操作的致命 bug。
  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

  // baseLoop 用户定义的loop
  EventLoop *loop_;

  const std::string ipPort_;
  const std::string name_;

  // 运行种子mainLoop监听新连接事件
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> threadPool_;

  // 有新连接时的回调
  ConnectionCallback connectionCallback_;
  // 有读写消息时的回调
  MessageCallback messageCallback_;
  // 消息发送完成以后的回调
  WriteCompleteCallback writeCompleteCallback_;

  // loop线程初始化的回调
  ThreadInitCallback threadInitCallback_;

  std::atomic_int started_;

  int nextConnId_;
  // 保存所有的连接
  ConnectionMap connections_;
};
