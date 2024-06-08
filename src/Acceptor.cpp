#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop,const std::string &ip,uint16_t port):loop_(loop)
{
     // 创建非阻塞的服务端套接字
    servsock_ = new Socket(createnonblocking());
    // 设置网络通信用的协议
    InetAddress servaddr(ip,port);
    // 设置网络中的端口复用、延时等属性的设置
    servsock_->setreuseaddr(true);
    servsock_->setreuseport(true);
    servsock_->settcpnodelay(true);
    servsock_->setkeepalive(true);
    // 绑定ip与端口号
    servsock_->bind(servaddr);
    // 服务端开始监听
    servsock_->listen();

    acceptchannel_ = new Channel(loop_,servsock_->fd());
    acceptchannel_->setreadcallback(std::bind(&Channel::newconnection,acceptchannel_,servsock_));
    acceptchannel_->enablereading(); // 让epoll_wait()监视servchannel的读事件。
}

Acceptor::~Acceptor()
{
    delete servsock_;
    delete acceptchannel_;
}