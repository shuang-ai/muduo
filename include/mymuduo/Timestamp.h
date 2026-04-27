#pragma once   // 防止头文件重复包含（效果等同于 include guards）

#include <iostream>   // 虽然未直接使用，但可能用于调试输出（如示例中的 main 函数）
#include <string>     // 使用 std::string 作为 toString() 的返回类型

/**
 * 时间戳类
 * 
 * 功能：表示一个时间点，内部存储自 1970-01-01 00:00:00 以来的秒数（精度仅到秒）。
 *       提供获取当前时间、转换为可读字符串的方法。
 * 
 * 注意：原版 muduo 中的 Timestamp 使用微秒级精度（int64_t 存储微秒数），
 *       但此简化版使用了 time(NULL) 返回的秒数，因此实际精度为秒。
 */
class Timestamp
{
public:
    /**
     * 默认构造函数
     * 创建一个时间为 0 的时间戳（即 1970-01-01 00:00:00）
     */
    Timestamp();

    /**
     * 使用从纪元开始的秒数构造时间戳
     * @param microSecondsSinceEpoch 自 1970-01-01 00:00:00 以来的秒数
     * 
     * 注意：参数名虽然叫 microSecondsSinceEpoch，但实际存储的是秒数，
     *       可能是为了与 muduo 原版保持命名一致，但实现简化了。
     */
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    /**
     * 获取当前时间的时间戳对象
     * @return 当前时刻的 Timestamp 对象
     */
    static Timestamp now();

    /**
     * 将时间戳转换为格式化的日期时间字符串
     * @return 字符串，格式为 "年/月/日 时:分:秒"，例如 "2025/03/20 14:30:25"
     */
    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;   // 存储自 1970-01-01 00:00:00 以来的秒数（命名保留了微秒字样，但实际是秒）
};