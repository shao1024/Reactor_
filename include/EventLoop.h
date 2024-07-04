#pragma once
#include "Epoll.h"
#include <functional>
#include <memory>

class Channel;
class Epoll;

class EventLoop
{
private:
    std::unique_ptr<Epoll> ep_;
    // epoll_wait()超时的回调函数 回调TcpServer::epolltimeout
    std::function<void(EventLoop*)> epolltimeoutcallback_;
public:
    EventLoop();
    ~EventLoop();

    // 运行事件循环。
    void run();
    
    // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void updatechannel(Channel *ch);
    // 从黑树上删除channel
    void removechannel(Channel *ch);
    // 设置epoll_wait()超时的回调函数。
    void setepolltimeoutcallback(std::function<void(EventLoop*)> fn);

};
