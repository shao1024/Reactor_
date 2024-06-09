#include "TcpServer.h"
#include <unistd.h>

TcpServer::TcpServer(const std::string &ip, const uint16_t port)
{
    acceptor_ = new Acceptor(&loop_,ip,port);
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete acceptor_;

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


    printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
    // 把conn存放map容器中
    conns_[conn->fd()] = conn;
}

// 关闭客户端的连接，在Connection类中回调此函数
void TcpServer::closeconnection(Connection* conn)
{
    printf("client(eventfd=%d) disconnected.\n",conn->fd());
    //::close(conn->fd());
    conns_.erase(conn->fd());
    // 会一层一层最后关闭对应的套接字
    delete conn;
}

// 客户端的连接错误，在Connection类中回调此函数
void TcpServer::errorconnection(Connection* conn)
{
    printf("client(eventfd=%d) error.\n",conn->fd());
    //::close(conn->fd());
    conns_.erase(conn->fd());
    // 会一层一层最后关闭对应的套接字
    delete conn;
}
