#pragma once
#include "InetAddress.h"

// 处理连上来的客户端，创建一个非阻塞的客户端socket,返回套接字
int createnonblocking();

// socket类，封装了socket一整套的连接过程
class Socket
{
private:
    // Socket持有的fd，在构造函数中传进来。
    const int fd_;
public:
    Socket(int fd);
    ~Socket();

    // 返回套接字
    int fd() const;
    // 设置SO_REUSEADDR选项，允许重复使用本地地址（端口）
    void setreuseaddr(bool on);
    // 设置SO_REUSEPORT选项，允许多个套接字绑定到同一个端口上
    void setreuseport(bool on);  
    // 设置TCP_NODELAY选项，禁用Nagle算法  
    void settcpnodelay(bool on);
    // 设置SO_KEEPALIVE选项，用于检测对端是否仍然在线，特别是在长连接中避免“半开连接”的问题。
    void setkeepalive(bool on);       

    // 绑定端口号和ip
    void bind(const InetAddress& servaddr);
    // 服务端开始监听
    void listen(int nn = 128);
    // 受理客户端连接，并将客户端的套接字设置为非阻塞
    int accept(InetAddress& clientaddr);

};