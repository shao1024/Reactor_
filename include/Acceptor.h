#pragma once

#include "Socket.h"
#include "EventLoop.h"
#include "Channel.h"
class Acceptor
{
private:
    // Acceptor对应的事件循环，在构造函数中传入
    EventLoop *loop_;
    // 服务端用于监听的socket，在构造函数中创建
    Socket *servsock_;
    // Acceptor对应的channel，在构造函数中创建。
    Channel *acceptchannel_;

    
public:
    Acceptor(EventLoop *loop,const std::string &ip,uint16_t port);
    ~Acceptor();
};


