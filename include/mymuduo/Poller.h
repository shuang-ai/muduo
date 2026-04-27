#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

// 前向声明防止重复依赖
class Channel;
class EventLoop;

// 处理IO多路复用核心模块
class Poller : noncopyable{
public:
  // 定义事件处理的类型
  using ChannelList=std::vector<Channel*>;
  Poller(EventLoop* loop);
  virtual ~Poller() = default;

  // 给所有的IO多路复用统一的接口
  // 等待事件
  virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels)=0;
  // 注册/更新事件
  virtual void updateChannel(Channel *channel) = 0;
  // 移除事件
  virtual void removeChannel(Channel *channel) = 0;

  // 判断channel是否在Poller中
  bool hasChannel(Channel* channel) const;

  // 创建一个适合当前操作系统的 Poller 对象
  static Poller* newDefaultPoller(EventLoop* loop);
  
protected:
  // sockfd : sockfd所对应的channel通道类型
  using ChannelMap=std::unordered_map<int, Channel*>;
  ChannelMap channels_;
private:
  // 定义Poller所属的事件循环EventLoop
  EventLoop *ownerLoop_; 
};