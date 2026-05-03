#include "EventLoop.h"
#include "Channel.h"
#include "Logger.h"
#include "Poller.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/eventfd.h>
#include <unistd.h>
#include <vector>
// 防止一个线程创建多个EventLoop   thread_local

__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd,用来notify唤醒subReactor处理新来的channel
// 只在一个 .cc 文件中使用，且是辅助函数
int createEventfd() {
  // 非阻塞，exec自动关闭
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

  if (evtfd < 0) {
    LOG_FATAL("eventfd error:%d \n", errno);
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()), wakeupChannel_(new Channel(this, wakeupFd_)) {

  LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);

  if (t_loopInThisThread) {
    LOG_FATAL("Another EventLoop %p exists in this thread %d \n",
              t_loopInThisThread, threadId_);
  } else {
    t_loopInThisThread = this;
  }

  // wakeupChannel_ 设置为读事件，是因为其他线程通过向 wakeupFd_ 写入数据来唤醒
  // EventLoop， 而写入操作会触发 wakeupFd_ 的可读事件，从而让 epoll_wait 返回。
  // 设置wakeupfd的事件类型以及发生事件后的回调操作
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  // 每一个eventloop都将监听wakeupchannel的EPOLLIN读事件
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  looping_ = true;
  quit_ = false;
  LOG_INFO("EventLoop %p start looping \n", this);

  while (!quit_) {
    activeChannels_.clear();
    // poller_监听哪些事件发生了
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (auto &channel : activeChannels_) {
      // 然后上报给channel处理
      channel->handleEvent(pollReturnTime_);
    }
    // 执行其他线程通过 queueInLoop/runInLoop 投递的跨线程任务
    // 典型场景：mainLoop 将新连接分配给 subLoop 时，需要唤醒 subLoop
    // 并执行分配回调
    doPendingFunctors();
  }
  LOG_INFO("EventLoop %p stop looping. \n", this);
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  // 因为如果 quit() 是在其他线程调用的，
  // EventLoop 可能正在 poll() 中阻塞，
  // 需要唤醒它，让它立即检查 quit_ 标志并退出。
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb) {
  // 在当前的loop线程中，执行cb
  if (isInLoopThread()) {
    cb();
  } else {
    // 在非当前loop线程中执行cb，就需要唤醒loop所在线程，执行cb
    queueInLoop(cb);
  }
}

// 把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // 唤醒相应的，需要执行上面回调操作的loop的线程了
    // 若不在当前的线程则需要唤醒loop所在的线程
    // 或者 loop 线程正在执行上一批回调（此时新加的任务不会被立即执行，需要唤醒让它再执行一轮）
    if (!isInLoopThread() || callingPendingFunctors_) 
    {
        wakeup(); // 唤醒loop所在线程
    }
}


void EventLoop::handleRead() {
  uint64_t one = 1;
  // wakeupFd_ 是通过 eventfd 创建的文件描述符，
  // 内核为它维护一个8 字节的计数器。
  // write(wakeupFd_, &n, 8)	计数器 增加 n
  // read(wakeupFd_, &buf, 8)
  // 返回计数器的值，并将计数器清零
  // 当计数器 > 0 时，epoll_wait 会报告该 fd 可读。
  ssize_t n = read(wakeupFd_, &one, sizeof(one));

  if (n != sizeof(one)) {
    LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
  }
}

// EventLoop的方法 =》 Poller的方法
void EventLoop::updateChannel(Channel *channel) {
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
  return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  // 表示执行上一轮其他线程投递到我们这个任务队列pendingFunctors_的任务
  callingPendingFunctors_ = true;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // 交换回调函数
    functors.swap(pendingFunctors_);
  }

  for (const auto &functor : functors) {
    // 执行当前loop需要执行的回调操作
    functor();
  }
  callingPendingFunctors_ = false;
}

// 用来唤醒loop所在的线程的  向wakeupfd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
    }
}

