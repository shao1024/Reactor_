#pragma once

#include "Socket.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Connection.h"

class Acceptor
{
private:
    // Acceptor对应的事件循环，在构造函数中传入
    EventLoop* loop_;
    // 服务端用于监听的socket，在构造函数中创建
    Socket servsock_;
    // Acceptor对应的channel，在构造函数中创建。
    Channel acceptchannel_;
    // 处理新客户端连接请求的回调函数，将指向TcpServer::newconnection()
    std::function<void(std::unique_ptr<Socket>)> newconnectioncb_;

    
public:
    Acceptor(EventLoop* loop,const std::string &ip,uint16_t port);
    ~Acceptor();

    // 处理新客户端连接请求
    void newconnection();

    // 设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置
    void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);
};


