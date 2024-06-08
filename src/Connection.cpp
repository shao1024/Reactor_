#include "Connection.h"


Connection::Connection(EventLoop* loop,Socket *clientsock):loop_(loop),clientsock_(clientsock)
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    clientchannel_ = new Channel(loop_,clientsock_->fd());
    // 新生成的套接字均为接受消息的功能，使用onmessage作为回调函数，在执行onmessage函数时，隐含要求该类的指针，所以将当前channel作为参数传进去
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage,clientchannel_));
    clientchannel_->useet(); // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();// 让epoll_wait()监视clientchannel的读事件。
}

Connection::~Connection()
{
    delete clientsock_;
    delete clientchannel_;

}