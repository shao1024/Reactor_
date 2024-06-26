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

    // 回调EchoServer::HandleNewConnection()
    std::function<void(Connection*)> newconnectioncb_;
    // 回调EchoServer::HandleClose()
    std::function<void(Connection*)> closeconnectioncb_;
    // 回调EchoServer::HandleError()
    std::function<void(Connection*)> errorconnectioncb_;
    // 回调EchoServer::HandleMessage()   注意第二个参数采用的是传引用的方式，当message中的数据量大时，这种方式速度更快
    std::function<void(Connection*,std::string &message)> onmessagecb_;
    // 回调EchoServer::HandleSendComplete()
    std::function<void(Connection*)> sendcompletecb_;
    // 回调EchoServer::HandleTimeOut()
    std::function<void(EventLoop*)> epolltimeoutcb_;
    
    
    
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
    // 处理客户端的请求报文，在Connection类中回调此函数
    void onmessage(Connection *conn,std::string message);
    // 数据发送完成后，在Connection类中回调此函数
    void sendcomplete(Connection* conn);
    // epoll_wait()超时，在EventLoop类中回调此函数
    void epolltimeout(EventLoop* loop);

    void setnewconnectioncb(std::function<void(Connection*)> fn);
    void setcloseconnectioncb(std::function<void(Connection*)> fn);
    void seterrorconnectioncb(std::function<void(Connection*)> fn);
    void setonmessagecb(std::function<void(Connection*,std::string &message)> fn);
    void setsendcompletecb(std::function<void(Connection*)> fn);
    void setepolltimeoutcb(std::function<void(EventLoop*)> fn);



};




