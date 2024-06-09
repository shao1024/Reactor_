#include "Connection.h"


Connection::Connection(EventLoop* loop,Socket *clientsock):loop_(loop),clientsock_(clientsock)
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    clientchannel_ = new Channel(loop_,clientsock_->fd());
    // 新生成的套接字均为接受消息的功能，使用onmessage作为回调函数，在执行onmessage函数时，隐含要求该类的指针，所以将当前channel作为参数传进去
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage,clientchannel_));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->useet(); // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();// 让epoll_wait()监视clientchannel的读事件。
}

Connection::~Connection()
{
    delete clientsock_;
    delete clientchannel_;

}

// 返回客户端的fd
int Connection::fd() const
{
    return clientsock_->fd();
}

// 返回客户端的ip
std::string Connection::ip() const
{
    return clientsock_->ip();
}

// 返回客户端的port
uint16_t Connection::port() const
{
    return clientsock_->port();
}


// TCP连接关闭（断开）的回调函数，供Channel回调
void Connection::closecallback()
{
    // 使用上层Tcpserver的关闭回调函数
    closecallback_(this);
}


// TCP连接错误的回调函数，供Channel回调
void Connection::errorcallback()
{
    // 使用上层Tcpserver的错误回调函数
    errorcallback_(this);
}

// 设置关闭fd_的回调函数。
void Connection::setclosecallback(std::function<void(Connection*)> fn)
{
    closecallback_ = fn;
}

// 设置fd_发生了错误的回调函数。
void Connection::seterrorcallback(std::function<void(Connection*)> fn)
{
    errorcallback_ = fn;
}
