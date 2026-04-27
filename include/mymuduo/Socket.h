#pragma once

#include "noncopyable.h"

class InetAddress;

// 封装socket fd
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {}

    ~Socket();

    int fd() const { return sockfd_; }
    // 绑定IP地址和端口号
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress *peeraddr);


    // 半关闭写端：告诉对端“我不会再发数据了”，
    // 但仍然可以接收对端发来的数据。
    void shutdownWrite();

    /**
 * @brief 设置 socket 选项（封装 setsockopt 系统调用）
 * 
 * 以下四个函数分别对应 TCP 套接字的关键配置，通过 setsockopt() 修改内核行为。
 * 
 * setsockopt() 参数说明：
 *   - level:   SOL_SOCKET（通用层）或 IPPROTO_TCP（TCP 层）
 *   - optname: 具体的选项宏（SO_REUSEADDR、TCP_NODELAY 等）
 *   - optval:  开启(1)或关闭(0)
 * 
 * 使用建议：在 bind() 和 listen() 之前调用。
 */
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};