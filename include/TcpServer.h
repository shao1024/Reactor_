#pragma once

#include "Acceptor.h"
#include <map>

class TcpServer
{
private:
    // 一个TcpServer可以有多个事件循环，现在是单线程，暂时只用一个事件循环。
    EventLoop loop_;
    // 一个TcpServer只有一个Acceptor对象。
    Acceptor *acceptor_;
    // 一个TcpServer有多个Connection对象，存放在map容器中;int表示套接字以及其对应的connection
    std::map<int,Connection*> conns_;
    
public:
    TcpServer(const std::string &ip, const uint16_t port);
    ~TcpServer();

    // 运行事件循环
    void start();

    // 处理新客户端连接请求,创建connection便于tcpserver管理
    void newconnection(Socket* clientsock);

    // 关闭客户端的连接，在Connection类中回调此函数
    void closeconnection(Connection* conn);
    // 客户端的连接错误，在Connection类中回调此函数
    void errorconnection(Connection* conn);
};




