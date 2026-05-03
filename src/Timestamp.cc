#include "Timestamp.h"   // 包含类声明
#include <time.h>        // 使用 time() 获取当前时间，localtime() 转换为本地时间

/**
 * 默认构造函数实现
 * 将内部时间戳初始化为 0（表示 1970-01-01 00:00:00）
 */
Timestamp::Timestamp()
    : microSecondsSinceEpoch_(0)
{
}

/**
 * 使用给定的秒数构造时间戳
 * @param microSecondsSinceEpoch 自纪元开始的秒数
 */
Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{
}

/**
 * 获取当前时刻的时间戳
 * @return 当前时间的 Timestamp 对象
 * 
 * 注意：time(NULL) 返回自纪元以来的秒数，精度到秒。
 *       因此该实现只能提供秒级精度，而非微秒级。
 */
Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));   // 调用构造函数，传入当前秒数
}

/**
 * 将时间戳转换为可读的日期时间字符串
 * @return 格式化的字符串，例如 "2025/03/20 14:30:25"
 * 
 * 工作流程：
 *   1. 使用 localtime() 将秒数转换为 struct tm（本地时间）
 *   2. 用 snprintf 按照固定格式写入字符数组
 *   3. 返回 std::string 对象
 */
std::string Timestamp::toString() const
{
    char buf[128] = {0};          // 足够大的缓冲区，用于存放格式化后的字符串
    // 将内部存储的秒数转换为本地时间的结构体
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    
    // 格式化输出：年/月/日 时:分:秒
    // tm_year 是从 1900 年开始的年数，因此要加 1900
    // tm_mon 范围 0~11，因此要加 1
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,   // 年份
        tm_time->tm_mon + 1,       // 月份
        tm_time->tm_mday,          // 日
        tm_time->tm_hour,          // 小时
        tm_time->tm_min,           // 分钟
        tm_time->tm_sec);          // 秒钟
    
    return buf;   // 返回 std::string（利用隐式转换）
}

/**
 * 下面是测试代码（已被注释）
 * 如果需要测试，可以取消注释并编译运行
 */
/*
#include <iostream>
int main()
{
    std::cout << Timestamp::now().toString() << std::endl; 
    return 0;
}
*/