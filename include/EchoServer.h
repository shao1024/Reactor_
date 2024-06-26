#pragma once

#include "TcpServer.h"
#include "Connection.h"

/*
    EchoServer类：回显服务器
*/
class EchoServer
{
private:
    TcpServer tcpserver_;

public:
    EchoServer(const std::string &ip,const uint16_t port);
    ~EchoServer();

    // 启动服务
    void Start();
    // 处理新客户端连接请求，在TcpServer类中回调此函数
    void HandleNewConnection(Connection* conn);
    // 关闭客户端的连接，在TcpServer类中回调此函数
    void HandleClose(Connection* conn);
    // 客户端的连接错误，在TcpServer类中回调此函数
    void HandleError(Connection* conn);
    // 处理客户端的请求报文，在TcpServer类中回调此函数
    void HandleMessage(Connection* conn,std::string& message);
    // 数据发送完成后，在TcpServer类中回调此函数
    void HandleSendComplete(Connection* conn);
    // epoll_wait()超时，在TcpServer类中回调此函数
    void HandleTimeOut(EventLoop* loop);


};





