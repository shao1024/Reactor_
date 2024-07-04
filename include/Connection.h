#pragma once

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include <memory>
#include <atomic>

// 声明Connection类，以便using语句能找到类名
class Connection;
using spConnection = std::shared_ptr<Connection>;

// 继承一个模板类，使得返回的类指针也是一个shareed_ptr智能指针；方便进行内存管理
class Connection:public std::enable_shared_from_this<Connection>
{
private:
    // Connection对应的事件循环，在构造函数中传入
    const std::unique_ptr<EventLoop>& loop_;
    // 与客户端通讯的Socket
    std::unique_ptr<Socket> clientsock_;
    // Connection对应的channel，在构造函数中创建
    std::unique_ptr<Channel> clientchannel_;

    // 接收缓存区
    Buffer inputbuffer_;
    // 发送缓存区
    Buffer outputbuffer_;
    // 客户端连接是否已断开，如果已断开，则设置为true。
    std::atomic_bool disconnect_;

    // 关闭fd_的回调函数，将回调上层的TcpServer::closeconnection()。
    std::function<void(spConnection)> closecallback_;
    // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
    std::function<void(spConnection)> errorcallback_;
    // 处理接受到报文的回调函数
    std::function<void(spConnection,std::string&)> onmessagecallback_;
    // 发送数据完成后的回调函数，将回调TcpServer::sendcomplete()
    std::function<void(spConnection)> sendcompletecallback_;


public:
    Connection(const std::unique_ptr<EventLoop>& loop,std::unique_ptr<Socket> clientsock);
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
    void setclosecallback(std::function<void(spConnection)> fn);
    // 设置fd_发生了错误的回调函数。
    void seterrorcallback(std::function<void(spConnection)> fn);
    // 设置消息的回调函数
    void setonmessagecallback(std::function<void(spConnection,std::string&)> fn);
    // 发送数据完成后的回调函数
    void setsendcompletecallback(std::function<void(spConnection)> fn);

    // 发送数据
    void send(const char* data,size_t size);
};



