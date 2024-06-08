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

public:
    Connection(EventLoop* loop,Socket *clientsock);
    ~Connection();
};



