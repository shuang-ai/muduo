#pragma once
#include "Timestamp.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <utility>

// 前置声明，防止循环依赖
class EventLoop;
class Channel : noncopyable {
public:
  // 事件回调函数
  using EventCallback = std::function<void()>;
  // 读事件回调函数
  using ReadEventCallback = std::function<void(Timestamp)>;

  // loop表示管理这个通道的事件循环
  // fd来自三个地方
// 监听 socket	listenfd（Acceptor 中）	接受新连接
// 已连接 socket	connfd（TcpConnection 中）	读写数据
// eventfd	wakeupFd_（EventLoop 中）	跨线程唤醒
  Channel(EventLoop *loop, int fd);
  ~Channel();

  // 处理事件函数
  void handleEvent(Timestamp receiveTime);

  // 设置回调函数
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  // 防止Channel被删除后还继续调用，可以用weak_ptr检测该对象是否销毁
  void tie(const std::shared_ptr<void> &);

  // 获取基本类型
  int fd() const { return fd_; }
  int events() const { return events_; }

  int index(){return index_;}
  void set_index(int idx){index_=idx;}
  // 设置poller返回的具体事件
  void set_revents(int revt) { revents_ = revt; }

  EventLoop *ownerLoop() { return loop_; }

  void remove();

    // 返回fd当前的事件状态
    // 当前 Channel 有没有注册任何“感兴趣的事件”
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    // 当前 Channel 是否注册了“可写事件”
    bool isWriting() const { return events_ & kWriteEvent; }
    // 当前 Channel 是否注册了“可读事件”
    bool isReading() const { return events_ & kReadEvent; }


  // 设置fd相应的事件状态
  // 能够处理读事件
  void enableReading() {
    events_ |= kReadEvent;
    update();
  }

  // 停止处理读事件
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  // 能够处理写事件
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  // 停止处理写事件
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  // 停止所有事件
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

private:
  // 更新事件状态
  void update();
  // 通知Channel发生的具体事件
  void handleEventWithGuard(Timestamp receiveTime);

  // 定义静态常量
  // 没有事件或者不关心任何事件
  static const int kNoneEvent;
  // 读事件
  static const int kReadEvent;
  // 写事件
  static const int kWriteEvent;

  EventLoop *loop_; // 事件循环
  const int fd_;    // fd,Poller监听的对象
  int events_;      // 注册fd感兴趣的事件
  int revents_;     // poller返回的具体事件
  int index_;       // poller内部使用的类型

  std::weak_ptr<void> tie_; // 用来检验Channel对象是否被销毁
  // 设置tied_，可以在handleEvent避免每次都创建一次shared_ptr指针
  // 以及少一次tie_.lock操作
  bool tied_;

  // 具体函数
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};