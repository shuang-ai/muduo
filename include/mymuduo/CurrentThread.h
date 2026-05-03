#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // extern表示“声明”，不是“定义”，这个变量在别的文件中定义，你先别分配内存，链接时去找
    // __thread表示每个线程拥有该变量的独立副本，而不是全局变量
    extern __thread int t_cachedTid;

    void cacheTid();

    // 为什么这里用内联？
    inline int tid()
    {
        // __builtin_expect是告诉编译器：这个分支很少执行，优化时把常见路径放前面
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}