#pragma once

#include "Acceptor.h"

class TcpServer
{
private:
    // 一个TcpServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环。
    EventLoop loop_;
    // 一个TcpServer只有一个Acceptor对象。
    Acceptor *acceptor_;
    
public:
    TcpServer(const std::string &ip, const uint16_t port);
    ~TcpServer();

    void start();
};




