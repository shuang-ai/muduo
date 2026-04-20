#pragma once   // 防止头文件重复包含

#include <arpa/inet.h>    // 提供 inet_ntop、inet_addr 等网络地址转换函数
#include <netinet/in.h>   // 提供 sockaddr_in 结构体定义
#include <string>         // 使用 std::string 类型

/**
 * InetAddress 类：封装 socket 地址
 * 
 * 功能：将 C 语言的 sockaddr_in 结构体封装成 C++ 类，提供便捷的 IP 地址和端口操作。
 *       自动处理网络字节序和主机字节序的转换。
 * 
 * sockaddr_in 结构体（IPv4 地址）：
 *   - sin_family   : 地址族，AF_INET 表示 IPv4
 *   - sin_port     : 端口号（网络字节序）
 *   - sin_addr     : IP 地址（网络字节序）
 */
class InetAddress
{
public:
    /**
     * 构造函数（端口+IP）
     * @param port 端口号（主机字节序，如 8888）
     * @param ip   IP 地址字符串（如 "127.0.0.1" 或 "0.0.0.0"），默认为本地回环地址
     * 
     * explicit 关键字：防止隐式转换，例如 InetAddress addr = 8080; 这样的写法会被禁止
     * 默认参数：port = 0 表示让操作系统自动分配端口，ip = "127.0.0.1" 表示只监听本地
     */
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    
    /**
     * 构造函数（从 sockaddr_in 构造）
     * @param addr 已有的 sockaddr_in 结构体
     * 
     * 使用场景：accept() 接受新连接时，返回客户端的地址信息
     */
    explicit InetAddress(const sockaddr_in &addr)
        : addr_(addr)      // 直接拷贝地址结构体
    {}

    /**
     * 获取 IP 地址字符串（不含端口）
     * @return IP 地址，例如 "192.168.1.100"
     */
    std::string toIp() const;
    
    /**
     * 获取 IP:端口 格式的字符串
     * @return 例如 "192.168.1.100:8888"
     */
    std::string toIpPort() const;
    
    /**
     * 获取端口号（主机字节序）
     * @return 端口号，例如 8888
     */
    uint16_t toPort() const;

    /**
     * 获取底层 sockaddr_in 结构体的指针（只读）
     * 用于 bind()、connect()、accept() 等系统调用
     */
    const sockaddr_in* getSockAddr() const { return &addr_; }
    
    /**
     * 设置底层 sockaddr_in 结构体
     * 用于 accept() 后保存客户端地址
     */
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
    
private:
    sockaddr_in addr_;   // 底层 IPv4 地址结构体
};