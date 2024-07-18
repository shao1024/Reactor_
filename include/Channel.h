#pragma once

#include "Epoll.h"
#include "Socket.h"
#include "EventLoop.h"
#include <functional>
#include <memory>

class EventLoop;


class Channel
{
private:
    // Channel拥有的fd，Channel和fd是一对一的关系。
    int fd_ = -1;
    // Channel对应的epoll句柄，一个Epoll包含多个Channel；一个channel有唯一的Epoll句柄
    EventLoop* loop_;
    // Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD。
    bool inepoll_=false;
    // fd_需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
    uint32_t events_=0;
    // 当前fd_发生的事件  
    uint32_t revents_=0;
    // fd_读事件的回调函数(因为不知道函数应该是怎样的参数类型，所以先弄成没有参数的)，其中function<void()>声明一个无参数无返回值的函数
    // 将回调Acceptor::newconnection()，如果是clientchannel，将回调Channel::onmessage()。
    std::function<void()> readcallback_;   
    // 关闭fd_的回调函数，将回调上层的Connection::closecallback()
    std::function<void()> closecallback_;
    // fd_发生了错误的回调函数，将回调Connection::errorcallback()
    std::function<void()> errorcallback_;
    // fd_写事件的回调函数，将回调Connection::writecallback()
    std::function<void()> writecallback_;


public:
    Channel(EventLoop* loop,int fd);
    ~Channel();

    // 返回fd_成员。
    int fd();
    // 返回events_成员
    uint32_t events();
    // 返回revents_成员
    uint32_t revents();
    // 采用边缘触发。
    void useet();
    // 设置epoll_wait()监视fd_的读事件。
    void enablereading();
    // 取消读事件
    void disablereading();
    // 注册写事件
    void enablewriting();
    // 取消写事件
    void disablewriting();
    // 取消全部的事件
    void disableall(); 
    // 从事件循环中删除Channel
    void remove();


    // 设置inepoll_成员的值
    void setinepoll(bool inepoll);
    // 返回inepoll_成员
    bool inepoll();
    // 设置revents_成员，即记录当前fd发生的事件
    void setrevents(uint32_t ev);

    // 事件处理函数，epoll_wait()返回的时候，执行它。
    void handleevent();

    // 设置fd_读事件的回调函数
    void setreadcallback(std::function<void()> fn);
    // 设置关闭fd_的回调函数
    void setclosecallback(std::function<void()> fn);
    // 设置fd_发生了错误的回调函数
    void seterrorcallback(std::function<void()> fn);
    // 设置写事件的回调函数
    void setwritecallback(std::function<void()> fn);


};



