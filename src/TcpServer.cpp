#include "TcpServer.h"
#include <unistd.h>
#include <iostream>

TcpServer::TcpServer(const std::string &ip, const uint16_t port)
{
    acceptor_ = new Acceptor(&loop_,ip,port);
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
    loop_.setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete acceptor_;
    // 释放所有的Connection对象
    for (auto &aa:conns_)
    {
        delete aa.second;
    }
    
}

void TcpServer::start()
{
    loop_.run();

}

void TcpServer::newconnection(Socket* clientsock)
{
    // 保证connection在TcpSever中建立，便于管理
    Connection *conn=new Connection(&loop_,clientsock);
    // 设置Connection的回调函数，
    conn->setclosecallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage,this,std::placeholders::_1,std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete,this,std::placeholders::_1));

    //printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
    // 把conn存放map容器中
    conns_[conn->fd()] = conn;

    // 回调EchoServer::HandleNewConnection()
    if (newconnectioncb_) newconnectioncb_(conn);
}

// 关闭客户端的连接，在Connection类中回调此函数
void TcpServer::closeconnection(Connection* conn)
{
    if (closeconnectioncb_)
    {
       closeconnectioncb_(conn);
    }
    
    //printf("client(eventfd=%d) disconnected.\n",conn->fd());
    //::close(conn->fd());
    conns_.erase(conn->fd());
    // 会一层一层最后关闭对应的套接字
    delete conn;
}

// 客户端的连接错误，在Connection类中回调此函数
void TcpServer::errorconnection(Connection* conn)
{
    if (errorconnectioncb_)
    {
        errorconnectioncb_(conn);
    }
    
    //printf("client(eventfd=%d) error.\n",conn->fd());
    //::close(conn->fd());
    conns_.erase(conn->fd());
    // 会一层一层最后关闭对应的套接字
    delete conn;
}


// 处理客户端的请求报文，在Connection类中回调此函数
void TcpServer::onmessage(Connection *conn,std::string message)
{
    // 处理报文的一些业务
    /*
    message = "reply:" + message;
    // 计算报文大小
    int len = message.size();
    // 添加报文头部4字节，表示报文长度
    std::string tmpbuf((char*)&len,4);
    // 添加报文内容
    tmpbuf.append(message);
    //std::cout<< "here"<<std::endl;
    // 发送报文
    conn->send(tmpbuf.data(),tmpbuf.size());
    */
    if (onmessagecb_)
    {
        onmessagecb_(conn,message);
    }

}

// 数据发送完成后，在Connection类中回调此函数
void TcpServer::sendcomplete(Connection* conn)
{
    if (sendcompletecb_)
    {
        sendcompletecb_(conn);
    }
    
}

// epoll_wait()超时，在EventLoop类中回调此函数
void TcpServer::epolltimeout(EventLoop* loop)
{
    if (epolltimeoutcb_)
    {
        epolltimeoutcb_(loop);
    }

}



void TcpServer::setnewconnectioncb(std::function<void(Connection*)> fn)
{
    newconnectioncb_ = fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(Connection*)> fn)
{
    closeconnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(Connection*)> fn)
{
    errorconnectioncb_ = fn;
}

// 
void TcpServer::setonmessagecb(std::function<void(Connection*,std::string &message)> fn)
{
    onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(std::function<void(Connection*)> fn)
{
    sendcompletecb_ = fn;
}

void TcpServer::setepolltimeoutcb(std::function<void(EventLoop*)> fn)
{
    epolltimeoutcb_ = fn;
}
