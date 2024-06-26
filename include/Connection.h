#pragma once

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"

class Connection
{
private:
    // Connection对应的事件循环，在构造函数中传入
    EventLoop *loop_;
    // 与客户端通讯的Socket
    Socket *clientsock_;
    // Connection对应的channel，在构造函数中创建
    Channel *clientchannel_;

    // 接收缓存区
    Buffer inputbuffer_;
    // 发送缓存区
    Buffer outputbuffer_;

    // 关闭fd_的回调函数，将回调上层的TcpServer::closeconnection()。
    std::function<void(Connection*)> closecallback_;
    // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
    std::function<void(Connection*)> errorcallback_;
    // 处理接受到报文的回调函数
    std::function<void(Connection*,std::string)> onmessagecallback_;
    // 发送数据完成后的回调函数，将回调TcpServer::sendcomplete()
    std::function<void(Connection*)> sendcompletecallback_;


public:
    Connection(EventLoop* loop,Socket *clientsock);
    ~Connection();

    // 返回客户端的fd
    int fd() const;
    // 返回客户端的ip
    std::string ip() const;
    // 返回客户端的port
    uint16_t port() const;

    // 处理对端发过来的消息
    void onmessage();
    // TCP连接关闭（断开）的回调函数，供Channel回调
    void closecallback();
    // TCP连接错误的回调函数，供Channel回调
    void errorcallback();
    // 处理写事件的回调函数，供Channel回调
    void writecallback();

    // 设置关闭fd_的回调函数。
    void setclosecallback(std::function<void(Connection*)> fn);
    // 设置fd_发生了错误的回调函数。
    void seterrorcallback(std::function<void(Connection*)> fn);
    // 设置消息的回调函数
    void setonmessagecallback(std::function<void(Connection*,std::string)> fn);
    // 发送数据完成后的回调函数
    void setsendcompletecallback(std::function<void(Connection*)> fn);

    // 发送数据
    void send(const char* data,size_t size);
};



