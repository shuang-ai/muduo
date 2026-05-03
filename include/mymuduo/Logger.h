#pragma once

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>   // for strrchr

#include "noncopyable.h"

// 只保留文件名（去掉路径）
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// 定义日志的级别
enum LogLevel
{
    INFO,   // 普通信息
    ERROR,  // 错误信息
    FATAL,  // 严重错误
    DEBUG,  // 调试信息
};

// 日志类
class Logger : noncopyable
{
public:
    // 获取日志唯一实例（单例）
    static Logger& instance();

    // 设置日志级别
    void setLogLevel(int level);

    // 写日志
    void log(std::string msg);

private:
    int logLevel_;
};

#define LOG_INFO(logmsgFormat, ...)                                      \
    do                                                                   \
    {                                                                    \
        Logger &logger = Logger::instance();                             \
        logger.setLogLevel(INFO);                                        \
        char buf[1024] = {0};                                            \
        snprintf(buf, sizeof(buf), "[%s:%d][%s] " logmsgFormat,          \
                 __FILENAME__, __LINE__, __func__, ##__VA_ARGS__);        \
        logger.log(buf);                                                 \
    } while (0)

#define LOG_ERROR(logmsgFormat, ...)                                     \
    do                                                                   \
    {                                                                    \
        Logger &logger = Logger::instance();                             \
        logger.setLogLevel(ERROR);                                       \
        char buf[1024] = {0};                                            \
        snprintf(buf, sizeof(buf), "[%s:%d][%s] " logmsgFormat,          \
                 __FILENAME__, __LINE__, __func__, ##__VA_ARGS__);        \
        logger.log(buf);                                                 \
    } while (0)

#define LOG_FATAL(logmsgFormat, ...)                                     \
    do                                                                   \
    {                                                                    \
        Logger &logger = Logger::instance();                             \
        logger.setLogLevel(FATAL);                                       \
        char buf[1024] = {0};                                            \
        snprintf(buf, sizeof(buf), "[%s:%d][%s] " logmsgFormat,          \
                 __FILENAME__, __LINE__, __func__, ##__VA_ARGS__);        \
        logger.log(buf);                                                 \
        exit(-1);                                                        \
    } while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                                     \
    do                                                                   \
    {                                                                    \
        Logger &logger = Logger::instance();                             \
        logger.setLogLevel(DEBUG);                                       \
        char buf[1024] = {0};                                            \
        snprintf(buf, sizeof(buf), "[%s:%d][%s] " logmsgFormat,          \
                 __FILENAME__, __LINE__, __func__, ##__VA_ARGS__);        \
        logger.log(buf);                                                 \
    } while (0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif