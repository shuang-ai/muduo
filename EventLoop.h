#pragma once
#include "CurrentThread.h"
#include "Timestamp.h"
#include "noncopyable.h"
// #include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
class Channel;
class Poller;

//  时间循环类，包含Channel和Poller
class EventLoop : noncopyable {
public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  // 开启事件循环
  void loop();
  // 退出事件循环
  void quit();

  // 获取被epoll_wait捕获的时间
  Timestamp pollReturnTime() const;

  // 在当前loop中执行cb
  void runInLoop(Functor cb);
  // 把cb放入队列中，唤醒loop所在的线程，执行cb
  void queueInLoop(Functor cb);
  // 用来唤醒loop所在的线程的
  void wakeup();

    // EventLoop的方法 =》 Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);

    // 是否有这个事件
    bool hasChannel(Channel *channel);

    // 判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ ==  CurrentThread::tid(); }
    
private:
  void handleRead();
  // 执行回调
  void doPendingFunctors();
  using ChannelList = std::function<Channel*>;
  
  // 判断是否还在循环运行
  std::atomic_bool looping_;
  // 标识退出loop循环
  std::atomic_bool quit_;

  // 记录当前loop所在线程的id
  const pid_t threadId_;

  // poller返回发生事件的channels的时间点(epoll_wait 返回后，用户程序获取的当前时间)
  Timestamp pollReturnTime_; 
  // EventLoop独占并且拥有Poller
  std::unique_ptr<Poller> poller_;

  // 主要作用，当mainLoop获取一个新用户的channel，
  // 通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
  

};