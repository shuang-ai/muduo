#include "EPollPoller.h"
#include "Channel.h"
#include "Logger.h"
#include "Poller.h"
#include <cerrno>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>

// channel未添加到poller中
const int kNew = -1; // channel的成员index_ = -1
// channel已添加到poller中
const int kAdded = 1;
// channel从poller中删除
const int kDeleted = 2;

// EPOLL_CLOEXEC表示让 epoll fd 在运行新程序时自动关闭，防止资源泄漏
EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_FATAL("epoll_create error:%d\n", errno);
  }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

// 等待事件
Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  LOG_INFO("func=%s => fd total count:%lu \n", __FUNCTION__, channels_.size());

  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);

  // errno 是一个全局变量（实际上是宏），用于记录系统调用失败时的错误代码。
  int saveErrno = errno;
  // Timestamp::now() 内部可能调用系统函数（如 gettimeofday），会覆盖
  // errno。如果不保存，原始的 errno 就丢失了。
  Timestamp now(Timestamp::now());

  if (numEvents > 0) {
    LOG_INFO("%d events happened \n", numEvents);
    fillActiveChannels(numEvents, activeChannels);
    if (numEvents == static_cast<int>(events_.size())) {
      events_.resize(2 * events_.size());
    }
  } else if (numEvents == 0) {
    // 此时超时了
    LOG_DEBUG("%s timeout! \n", __FUNCTION__);
  } else {
    // 调用失败，需要检查 errno
    // EINTR 不是真正的错误，只是系统调用的中断，应该重试，而不是直接报错退出。
    if (saveErrno != EINTR) {
      errno = saveErrno;
      LOG_ERROR("EPollPoller::poll() err!");
    }
  }
  return now;
}

void EPollPoller::updateChannel(Channel *channel) {
  // 获取消息类型
  const int index = channel->index();
  LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__,
           channel->fd(), channel->events(), index);
  // 之前被删除过或者是新创建的
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      int fd = channel->fd();
      channels_[fd] = channel;
    }
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    // 此时说明channel在poller注册过了
    int fd = channel->fd();
    // 如果此时channel关闭了监听
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

// 从poller删除channel
void EPollPoller::removeChannel(Channel *channel) {
  // 先删除channel监听的那个文件描述符
  int fd = channel->fd();
  channels_.erase(fd);

  LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);
  int index = channel->index();
  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  // 注意！！！此时要把状态改为创建了但是没有添加到Poller中
  channel->set_index(kNew);
}

// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList *activeChannels) const {
  for (int i = 0; i < numEvents; ++i) {
    // 把events_的每一个都放在活跃的通道里面
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    //     struct epoll_event
    // {
    //   uint32_t events;	/* Epoll events */
    //   epoll_data_t data;	/* User data variable */
    // } __EPOLL_PACKED;
    // 位掩码它是一个 32 位的整数，每个 bit 代表一种事件类型
    // // 常见的事件宏定义（数值是 2 的幂，即只有一位是 1）
    // #define EPOLLIN     0x001   // 二进制: 0000 0000 0000 0001
    // #define EPOLLOUT    0x004   // 二进制: 0000 0000 0000 0100
    // #define EPOLLERR    0x008   // 二进制: 0000 0000 0000 1000
    // #define EPOLLHUP    0x010   // 二进制: 0000 0000 0001 0000
    // #define EPOLLRDHUP  0x2000  // 二进制: 0010 0000 0000 0000
    channel->set_revents(events_[i].events);
    activeChannels->emplace_back(channel);
  }
}

// 更新channel通道 epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel *channel) {
  epoll_event event;
  bzero(&event, sizeof(event));
  int fd = channel->fd();
  event.events = channel->events();
  event.data.fd = fd;
  event.data.ptr = channel;

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR("epoll_ctl del error:%d\n", errno);
    } else {
      LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
    }
  }
}
