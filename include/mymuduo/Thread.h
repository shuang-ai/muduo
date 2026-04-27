#include "noncopyable.h"
#include <atomic>
#include <functional>
#include <sched.h>
#include <string>
#include<memory>
#include <thread>
class Thread:noncopyable{
public:
  using ThreadFunc=std::function<void()>;
  // 用左值的常引用，直接绑定右值临时拷贝的对象，效率高
  // 如果不加const就会报错，因为临时对象一定是刚刚创建完就销毁了
  // 后续改不了，所以C++工程师就认为这是不合法的
  explicit Thread(ThreadFunc,const std::string &name=std::string());
  ~Thread();
  void start();
  void join();
  bool start() const{return started_;}
  pid_t tid()const{return tid_;}
  const std::string& name()const {return name_;}

  static int numCreated(){return numCreated_;}

private:
  bool started_;
  bool joined_;
  pid_t tid_;
  // 这样存放thread，否则该线程一创建就会启动
  std::shared_ptr<std::thread> thread_;

  // 线程处理任务
  ThreadFunc func_;
  // 线程名字
  std::string name_;
  // 线程数量
  static std::atomic_int32_t numCreated_;

  void setDefaultName();
};