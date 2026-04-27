#include "Thread.h"
#include "CurrentThread.h"
#include <cstdio>
#include <future>
#include <memory>
#include <semaphore.h>
#include <thread>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)),
      name_(name) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    // thread类提供的设置分离线程的方法
    thread_->detach();
  }
}

// // 一个thread对象记录是一个新线程的详细信息
// void Thread::start() {
//   started_ = true;

//   // 内部包含资源数量和等待队列
//   // 等待队列中放的是阻塞的线程
//   // 这个sem只是个操作底层消耗量的句柄
//   sem_t sem;
//   thread_ = std::make_shared<std::thread>([this, &sem]() {
//     // 获取线程tid值
//     // 多线程程序，应该要等待这个tid_获取完之后再执行
//     tid_ = CurrentThread::tid();
//     // v操作
//     sem_post(&sem);
//     // 处理线程函数
//     func_();
//   });
//   // 相当于p操作，等待tid_被获取到才被sem_post唤醒
//   sem_wait(&sem);
// }
void Thread::start() {
    started_ = true;
    std::promise<void> promise;
    auto future = promise.get_future();
    thread_ = std::shared_ptr<std::thread>(new std::thread([this, promise = std::move(promise)]() mutable {
        tid_ = CurrentThread::tid();
        promise.set_value();
        func_();
    }));
    future.wait();   // 等待子线程 set_value
}

void Thread::setDefaultName() {
  // 此时创建了一个新线程
  int num = ++numCreated_;
  if (name_.empty()) {
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "Thread%d", num);
    name_ = buf;
  }
}

void Thread::join() {
  joined_ = true;
  thread_->join();
}
