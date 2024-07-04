#pragma once

#include "Channel.h"
#include <sys/epoll.h>
#include <vector>
class Channel;

// 封装了一个Epoll类
class Epoll
{
private:
    // epoll_wait()返回事件数组的大小。
    static const int MaxEvents = 100;
    // epoll句柄，在构造函数中创建。
    int epollfd_ = -1;
    // 存放poll_wait()返回事件的数组，在构造函数中分配内存。
    epoll_event events_[MaxEvents];

public:
    Epoll();
    ~Epoll();

    // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void updatechannel(Channel *ch);
    // 从红黑树上删除channel
    void removechannel(Channel *ch);
    // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
    std::vector<Channel *> loop(int timeout = -1);
};



