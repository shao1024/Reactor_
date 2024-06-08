#pragma once
#include "Epoll.h"

class Channel;
class Epoll;

class EventLoop
{
private:
    Epoll *ep_;
public:
    EventLoop();
    ~EventLoop();

    // 运行事件循环。
    void run();
    
    // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void updatechannel(Channel *ch);

};
