#pragma once

#include <arpa/inet.h>
#include <netinet/tcp.h>  // TCP_NODELAY需要包含这个头文件。
#include <string>

// 封装好的地址协议类
class InetAddress
{
private:
    // 定义地址协议的结构体 
    struct sockaddr_in addr_;
public:
    // 重载的构造函数
    InetAddress();
    InetAddress(const std::string &ip,uint16_t port);
    InetAddress(const sockaddr_in addr);
    ~InetAddress();

    const char *ip() const;
    uint16_t port() const;
    const sockaddr *addr() const;
    void setaddr(sockaddr_in clientaddr);

};