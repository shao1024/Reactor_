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
    // 工作线程池 存放计算的业务
    ThreadPool threadpool_;

public:
    EchoServer(const std::string &ip,const uint16_t port, int subthreadnum=3, int workthreadnum=5);
    ~EchoServer();

    // 启动服务
    void Start();
    // 处理新客户端连接请求，在TcpServer类中回调此函数
    void HandleNewConnection(spConnection conn);
    // 关闭客户端的连接，在TcpServer类中回调此函数
    void HandleClose(spConnection conn);
    // 客户端的连接错误，在TcpServer类中回调此函数
    void HandleError(spConnection conn);
    // 处理客户端的请求报文，在TcpServer类中回调此函数
    void HandleMessage(spConnection conn,std::string& message);
    // 数据发送完成后，在TcpServer类中回调此函数
    void HandleSendComplete(spConnection conn);
    // epoll_wait()超时，在TcpServer类中回调此函数
    void HandleTimeOut(EventLoop* loop);

    // 处理客户端的请求报文，用于添加给工作线程池
    void Onmessage(spConnection conn,std::string& message);
};





