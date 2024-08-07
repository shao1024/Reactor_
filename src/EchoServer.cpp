#include "EchoServer.h"
#include <iostream>


EchoServer::EchoServer(const std::string &ip,const uint16_t port,int subthreadnum, int workthreadnum)
                :tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORKS")
{
    // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数
    tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection,this,std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleClose,this,std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError,this,std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage,this,std::placeholders::_1,std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete,this,std::placeholders::_1));
    //tcpserver_.setepolltimeoutcb(std::bind(&EchoServer::HandleTimeOut,this,std::placeholders::_1));
}

EchoServer::~EchoServer()
{

}


// 启动服务
void EchoServer::Start()
{
    tcpserver_.start();
}

// 停止服务
void EchoServer::Stop()
{
    // 停止工作线程
    threadpool_.stop();
    printf("工作线程已停止。\n");

    // 停止IO线程（事件循环）
    tcpserver_.stop();
}

// 处理新客户端连接请求，在TcpServer类中回调此函数
void EchoServer::HandleNewConnection(spConnection conn)
{
    printf ("new connection(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());

    // 根据业务的需求，在这里可以增加其它的代码

}

// 关闭客户端的连接，在TcpServer类中回调此函数
void EchoServer::HandleClose(spConnection conn)
{
    printf ("connection closed(fd=%d,ip=%s,port=%d).\n",conn->fd(),conn->ip().c_str(),conn->port());
    // std::cout << "EchoServer conn closed." << std::endl;
    // 根据业务的需求，在这里可以增加其它的代码

}

// 客户端的连接错误，在TcpServer类中回调此函数
void EchoServer::HandleError(spConnection conn)
{
    // std::cout << "EchoServer conn error." << std::endl;
    // 根据业务的需求，在这里可以增加其它的代码
}

// 处理客户端的请求报文，在TcpServer类中回调此函数
void EchoServer::HandleMessage(spConnection conn,std::string& message)
{
    
    // 工作线程池为空时
    if (threadpool_.size() == 0)
    {   
        // 如果没有工作线程，则直接在IO线程中处理业务
        Onmessage(conn,message);
    }
    else
    {
        // 把业务添加到了线程池的任务队列中
        threadpool_.addtask(std::bind(&EchoServer::Onmessage,this,conn,message));

    } 
}

// 处理客户端的请求报文，用于添加给线程池
void EchoServer::Onmessage(spConnection conn,std::string& message)
{
    
    message = "reply:" + message;
    
    conn->send(message.data(),message.size());
}

// 数据发送完成后，在TcpServer类中回调此函数
void EchoServer::HandleSendComplete(spConnection conn)
{
    std::cout << "Message send complete." << std::endl;
    // 根据业务的需求，在这里可以增加其它的代码
}

// epoll_wait()超时，在TcpServer类中回调此函数
void EchoServer::HandleTimeOut(EventLoop* loop)
{
    std::cout << "EchoServer timeout." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码
}
