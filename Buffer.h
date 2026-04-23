#pragma once

#include <vector>
#include <string>
#include <algorithm>

// ============================================================================
// 网络库底层的缓冲区类
// 设计思路：使用 vector<char> 作为动态缓冲区，用三个指针（索引）管理读写位置
// 
// 内存布局：
        // Buffer 内存布局
        // =========================================================
        
        // <- kCheapPrepend -> <---- readable ---->
        // (固定8字节)         (已收到未处理的数据)
        
        // +------------------+------------------+------------------+
        // |                  |                  |                  |
        // |    预留区         |    可读数据区     |    可写空闲区     |
        // |  (prependable)   |   (readable)     |   (writable)     |
        // |                  |                  |                  |
        // +------------------+------------------+------------------+
        // ^                  ^                  ^                  ^
        // |                  |                  |                  |
        // |                  |                  |                  |
        // 0              readerIndex       writerIndex        buffer.size()
        
        // =========================================================
        
        // 各区域大小计算：
        // - prependableBytes() = readerIndex
        // - readableBytes()    = writerIndex - readerIndex
        // - writableBytes()    = buffer.size() - writerIndex
        
        // =========================================================
        
        // 示例（假设 buffer.size() = 1024, readerIndex = 8, writerIndex = 100）：
        
        // [0-7]        : 预留区（空闲，可用来写协议头）
        // [8-99]       : 可读数据区（有 92 字节待处理）
        // [100-1023]   : 可写空闲区（有 924 字节可用）
class Buffer
{
public:
    // ========================================================================
    // 一、常量定义
    // ========================================================================
    static const size_t kCheapPrepend = 8;   // 预留字节数（用于添加头部信息，如消息长度）
    static const size_t kInitialSize = 1024; // 初始缓冲区大小（1KB）

    // ========================================================================
    // 二、构造函数
    // ========================================================================
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)  // 预分配：预留区 + 初始容量
        , readerIndex_(kCheapPrepend)           // 读指针指向预留区末尾
        , writerIndex_(kCheapPrepend)           // 写指针指向预留区末尾（起始时无数据）
    {}

    // ========================================================================
    // 三、容量查询函数（内联，性能关键）
    // ========================================================================
    
    // 可读字节数 = 写指针 - 读指针
    size_t readableBytes() const 
    {
        return writerIndex_ - readerIndex_;
    }

    // 可写字节数 = 总大小 - 写指针
    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    // 预留区可复用字节数 = 读指针（读指针前面的空间都可以用来写）
    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    // ========================================================================
    // 四、读操作（从缓冲区取出数据）
    // ========================================================================
    
    // 返回可读数据的起始地址（读指针位置）
    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    // 消费 len 字节数据（移动读指针）
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            // 只消费一部分，读指针后移
            readerIndex_ += len;
        }
        else
        {
            // 消费所有数据，直接重置
            retrieveAll();
        }
    }

    // 消费所有数据（重置读/写指针到初始位置）
    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 将所有可读数据转成 string 并消费
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    // 读取 len 字节数据转成 string 并消费
    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);  // 从 peek() 位置拷贝 len 字节
        retrieve(len);                     // 移动读指针，表示已消费
        return result;
    }

    // ========================================================================
    // 五、写操作（向缓冲区添加数据）
    // ========================================================================
    
    // 返回可写区域的起始地址
    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    // 确保有足够的可写空间，如果不足则扩容或腾挪数据
    void ensureWriteableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);  // 扩容或整理内存
        }
    }

    // 追加数据到缓冲区
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());  // 拷贝数据到可写区域
        writerIndex_ += len;                         // 写指针后移
    }

    // ========================================================================
    // 六、网络 I/O 操作（与 socket 交互）
    // ========================================================================
    
    // 从 socket fd 读取数据到缓冲区（非阻塞读）
    ssize_t readFd(int fd, int* saveErrno);
    
    // 将缓冲区数据写入 socket fd（非阻塞写）
    ssize_t writeFd(int fd, int* saveErrno);

private:
    // ========================================================================
    // 七、私有辅助函数
    // ========================================================================
    
    // 返回底层数组的起始地址（vector 的第一个元素地址）
    char* begin()
    {
        return &*buffer_.begin();
    }
    
    const char* begin() const
    {
        return &*buffer_.begin();
    }

    // 扩容或整理内存，确保有 len 字节的可写空间
    void makeSpace(size_t len)
    {
        // 情况1：当前总空闲（可写+预留区）不够，直接扩容
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        // 情况2：空闲够，但分散在预留区和可写区，需要把数据搬到前面腾出连续空间
        else
        {
            size_t readable = readableBytes();
            // 将可读数据搬到预留区后面（从 kCheapPrepend 位置开始放）
            std::copy(begin() + readerIndex_, 
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            // 重置指针：读指针回到预留区末尾，写指针指向数据末尾
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    // ========================================================================
    // 八、成员变量
    // ========================================================================
    std::vector<char> buffer_;   // 底层存储（动态数组）
    size_t readerIndex_;         // 读指针位置（可读数据的起始位置）
    size_t writerIndex_;         // 写指针位置（可写数据的起始位置）
};