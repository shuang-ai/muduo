#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;   

    void cacheTid()
    {
        if (t_cachedTid == 0)
        {
            // 是 POSIX 定义的整数类型，用于表示进程 ID 或线程 ID
            // syscall是Linux 系统调用接口，直接进入内核
            // SYS_gettid是系统调用号，表示“获取当前线程 ID”
            // 通过linux系统调用，获取当前线程的tid值
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}