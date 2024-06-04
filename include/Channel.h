#pragma once

#include "Epoll.h"
#include "Socket.h"
class Epoll;

class Channel
{
private:
    // Channel拥有的fd，Channel和fd是一对一的关系。
    int fd_ = -1;
    // Channel对应的epoll句柄，一个Epoll包含多个Channel；一个channel有唯一的Epoll句柄
    Epoll *ep_ = nullptr;
    // Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD。
    bool inepoll_=false;
    // fd_需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
    uint32_t events_=0;
    // 当前fd_发生的事件  
    uint32_t revents_=0;
    // Channel是否是用于监听的套接字
    bool islisten_ = false;    

public:
    Channel(Epoll* ep,int fd,bool islisten);
    ~Channel();

    // 返回fd_成员。
    int fd();
    // 返回events_成员
    uint32_t events();
    // 返回revents_成员
    uint32_t revents();
    // 返回inepoll_成员
    bool inepoll();
    // 采用边缘触发。
    void useet();
    // 设置epoll_wait()监视fd_的读事件。
    void enablereading();
    // 把inepoll_成员的值设置为true。
    void setinepoll();
    // 设置revents_成员，即记录当前fd发生的事件
    void setrevents(uint32_t ev);

    // 事件处理函数，epoll_wait()返回的时候，执行它。
    void handleevent(Socket *servsock);
};



