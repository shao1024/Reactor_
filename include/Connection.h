#pragma once

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"


class Connection
{
private:
    // Connection对应的事件循环，在构造函数中传入
    EventLoop *loop_;
    // 与客户端通讯的Socket
    Socket *clientsock_;
    // Connection对应的channel，在构造函数中创建
    Channel *clientchannel_;

    // 关闭fd_的回调函数，将回调上层的TcpServer::closeconnection()。
    std::function<void(Connection*)> closecallback_;
    // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
    std::function<void(Connection*)> errorcallback_;


public:
    Connection(EventLoop* loop,Socket *clientsock);
    ~Connection();

    // 返回客户端的fd
    int fd() const;
    // 返回客户端的ip
    std::string ip() const;
    // 返回客户端的port
    uint16_t port() const;

    // TCP连接关闭（断开）的回调函数，供Channel回调
    void closecallback();
     // TCP连接错误的回调函数，供Channel回调
    void errorcallback();

    // 设置关闭fd_的回调函数。
    void setclosecallback(std::function<void(Connection*)> fn);
    // 设置fd_发生了错误的回调函数。
    void seterrorcallback(std::function<void(Connection*)> fn);


};



