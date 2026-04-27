#pragma once
#include "Channel.h"
#include "Poller.h"
#include <sys/epoll.h>
class Channel;

class EPollPoller : public Poller{
public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller();
    // 等待事件
  virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;
  // 注册/更新事件
  virtual void updateChannel(Channel *channel) override;
  // 移除事件
  virtual void removeChannel(Channel *channel) override;

private:
  // 事件列表的初始大小
  static const int kInitEventListSize = 16;
  // 填写活跃连接
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  // 更新channel通道
  void update(int operation, Channel *channel);

  // 返回的事件数组类型
  using EventList=std::vector<epoll_event>;
  int epollfd_;
  // “就绪”是内核说的（原始数据），“活跃”是程序转换后的（Channel 对象）。
  // 记录就绪的事件
  EventList events_; 

};
