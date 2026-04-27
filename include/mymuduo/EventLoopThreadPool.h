#pragma once
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class EventLoopThreadPool : noncopyable {
public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  // EventLoop *baseLoop这个是主线程专门用来监听客户端的连接
  // 不是线程池内部用线程处理的事件
  // const std::string &nameArg是线程池的名字，分别日志调试
  // 线程池内的线程主要处理IO
  EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
  ~EventLoopThreadPool() = default;

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }

  void start(const ThreadInitCallback &cb = ThreadInitCallback());

      // 如果工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();

    // 获取所有的事件循环(除了主线程的baseLoop)
    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }
    const std::string name() const { return name_; }

private:
  EventLoop* baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  // 下一个要分配的 subLoop 的索引
  // subLoop是EventLoop的对象
  int next_;  
  // 存放EventLoopThread线程
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};