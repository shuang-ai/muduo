#include "InetAddress.h"   // 包含类声明
#include <strings.h>       // 提供 bzero 函数（将内存块清零）
#include <string.h>        // 提供 strlen、sprintf 等字符串函数

/**
 * 构造函数实现：使用指定的端口和 IP 地址初始化 sockaddr_in 结构体
 * @param port 端口号（主机字节序）
 * @param ip   IP 地址字符串（如 "127.0.0.1"）
 * 
 * 工作流程：
 *   1. 将地址结构体全部清零
 *   2. 设置地址族为 IPv4（AF_INET）
 *   3. 将端口号从主机字节序转换为网络字节序
 *   4. 将 IP 字符串转换为网络字节序的整数
 */
InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof addr_);              // 将整个结构体清零，防止残留数据
    addr_.sin_family = AF_INET;               // 设置地址族：IPv4
    addr_.sin_port = htons(port);             // 端口号：主机序 → 网络序（大端）
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());  // IP 字符串 → 网络字节序整数
    // inet_addr() 返回的已经是网络字节序，直接赋值
}

/**
 * 获取 IP 地址字符串（不含端口）
 * @return 例如 "192.168.1.100"
 * 
 * 工作流程：使用 inet_ntop 将二进制 IP 地址转换为点分十进制字符串
 *   inet_ntop 是线程安全的（相比 inet_ntoa）
 */
std::string InetAddress::toIp() const
{
    char buf[64] = {0};                       // 足够大的缓冲区存放 IP 字符串
    // inet_ntop是网络字节序的二进制 IP 地址转换为人类可读的点分十进制字符串表
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    // 参数：地址族、二进制地址指针、输出缓冲区、缓冲区大小
    return buf;                               // 返回 std::string（利用隐式转换）
}

/**
 * 获取 IP:端口 格式的字符串
 * @return 例如 "192.168.1.100:8888"
 * 
 * 工作流程：
 *   1. 先用 inet_ntop 将 IP 转为字符串，存入 buf
 *   2. 获取 buf 当前长度 end
 *   3. 从 buf+end 位置开始拼接 ":" + 端口号
 *   4. 返回完整的字符串
 */
std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};                       // 缓冲区存放 "IP:port"
    
    // 第一步：将 IP 地址写入缓冲区
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    
    // 第二步：获取已写入字符串的长度（用于追加端口号）
    size_t end = strlen(buf);                 // end 指向 buf 中字符串的末尾位置
    
    // uint16_t相当于 unsigned short
    // 第三步：将端口号转换为网络字节序后，再转回主机字节序（因为要显示给用户看）
    uint16_t port = ntohs(addr_.sin_port);    // 网络序 → 主机序
    
    // 第四步：从 buf+end 位置开始追加端口号
    // sprintf 返回写入的字符数（不包括结尾的 \0）
    sprintf(buf + end, ":%u", port);          // 注意：buf+end 是字符串末尾的指针
    
    return buf;                               // 返回 "IP:port" 格式的字符串
}

/**
 * 获取端口号（主机字节序）
 * @return 端口号，例如 8888
 * 
 * 因为 sockaddr_in 中存储的端口是网络字节序（大端），
 * 需要调用 ntohs 转换为主机字节序后才能使用。
 */
uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);             // 网络序 → 主机序
}

/**
 * 下面是测试代码（已被注释）
 * 如果需要测试，可以取消注释并编译运行
 */
/*
#include <iostream>
int main()
{
    InetAddress addr(8888);                   // 创建地址对象，默认 IP 为 127.0.0.1
    std::cout << addr.toIpPort() << std::endl; // 输出 "127.0.0.1:8888"
    
    InetAddress addr2(8080, "0.0.0.0");       // 监听所有网卡
    std::cout << addr2.toIpPort() << std::endl; // 输出 "0.0.0.0:8080"
    
    return 0;
}
*/