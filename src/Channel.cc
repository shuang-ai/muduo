#include "Channel.h"
#include <memory>
#include<sys/epoll.h>
#include"Logger.h"
#include "EventLoop.h"

const int Channel::kNoneEvent = 0;
// EPOLLIN表示可读
// EPOLLPRI表示紧急可读
// 表示可写
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), tied_(false), index_(-1) {}

Channel::~Channel() {}

// 当新的tcpconnection连接的时候调用这个函数
// 这个obj是指向tcpconnection的
void Channel::tie(const std::shared_ptr<void> &obj) {
  tied_ = true;
  tie_ = obj;
}

// 改变Channel的事件
// 先触发事件循环中的update，然后再调用poller内的update
// 最后再调用epoll_ctl来改变事件
void Channel::update() 
{ loop_->updateChannel(this); }

// 在当前的channel所属的EventLoop中删除这个channel
// 每个 Channel 只属于一个 EventLoop（一对一关系）
void Channel::remove() 
{ loop_->removeChannel(this); }

// 处理事件函数
void Channel::handleEvent(Timestamp receiveTime) 
{
  // 若此时对象还在
  if(tied_){
    // 获取当前弱指针指向的tcpconnction对象
    std::shared_ptr<void> guard = tie_.lock();
    if(guard){
      // 此时对象还没有销毁
      handleEventWithGuard(receiveTime);
    }
  }else{
    handleEventWithGuard(receiveTime);
  }
}

// 根据poller通知的channel发生的具体事件， 由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime){
  // 打印日志
  LOG_INFO("channel handleEvent revents:%d\n", revents_);
  // 处理关闭回调
  // POLLHUP 表示“挂起事件”（Hang Up），即对端关闭了连接
  //   对端关闭，但还有数据可读	✅	✅	应该先读数据，再关闭
  //   对端关闭，没有数据了	✅	❌	直接关闭，不需要读

  if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
  {
    // 防止传入没有复制的函数对象导致直接崩溃
    if(closeCallback_){
      closeCallback_();
    }
  }

  // 错误回调
  if(revents_ & EPOLLERR){
    if(errorCallback_){
      errorCallback_();
    }
  }

  // 读回调
  if(revents_ & (kReadEvent)){
    if(readCallback_){
      readCallback_(receiveTime);
    }
  }

  // 写回调
  if(revents_ & (kWriteEvent)){
    if(writeCallback_){
      writeCallback_();
    }
  }
}

