#include "EventLoopThread.h"
#include "EventLoop.h"
#include <cstddef>


EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, 
        const std::string &name)
        : loop_(nullptr)
        , exiting_(false)
        , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
        , mutex_()
        , cond_()
        , callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        // 必须要等待子线程执行完后再销毁
        // 若调用detach则子线程访问已经销毁的资源会造成崩溃
        thread_.join();
    }
}

// 对外接口，外面可以拿到loop_
EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); // 启动底层的新线程

    // EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock,[this](){return loop_!=nullptr;});
        // loop = loop_;
    }
    // 返回这个地址的值
    // 虽然这个loop对象销毁了，但是指针指向的值没有变
    // return loop;
    return loop_;
}

// 下面这个方法，实在单独的新线程里面运行的
void EventLoopThread::threadFunc()
{
  // 创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread
    EventLoop loop; 

    // 如果用户设置了连接回调函数，则调用它
    if (callback_)
    {
        callback_(&loop);
    }

    // 给loop_赋值，然后唤醒startLoop内的wait
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop(); // EventLoop loop  => Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}