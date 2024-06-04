#include "Epoll.h"
#include <unistd.h>
#include <string.h>
Epoll::Epoll()
{
    // 创建epoll句柄（红黑树）
    if((epollfd_ = epoll_create(1)) == -1){
        printf("epoll_create() failed(%d).\n",errno); exit(-1);
    }

}

Epoll::~Epoll()
{
    ::close(epollfd_);
}

void Epoll::updatechannel(Channel *ch)
{
    // 声明epoll事件的数据结构。
    epoll_event ev;
    ev.data.ptr = ch; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = ch->events();  // 指定监听的事件

    // 使用不同的宏将套接字的信息Channel写入epoll句柄
    if (ch->inepoll()) // channel已经在红黑树上了，
    {
         
        if (epoll_ctl(epollfd_,EPOLL_CTL_MOD,ch->fd(),&ev) == -1)
        {
           perror("epoll_ctl() failed.\n"); exit(-1);
        }
 
    }
    else
    {
        if (epoll_ctl(epollfd_,EPOLL_CTL_ADD,ch->fd(),&ev) == -1)
        {
            perror("epoll_ctl() failed.\n"); exit(-1);
        }
        ch->setinepoll();
    }
}

std::vector<Channel *> Epoll::loop(int timeout)
{
    // 存放epoll_wait()返回的事件。
    std::vector<Channel*> result;

    bzero(events_,sizeof(events_));
    // 等待监视的fd有事件发生。
    int infds = epoll_wait(epollfd_,events_,MaxEvents,timeout);
    
    // 返回失败。
    if(infds <0){
        perror("epoll_wait");exit(-1);
    }
    // 超时
    if(infds == 0){
        perror("time out"); return result;
    }

    // 如果infds>0，表示有事件发生的fd的数量。
    for(int i=0; i<infds; i++){
        Channel *ch = (Channel *)events_[i].data.ptr;
        ch->setrevents(events_[i].events);
        result.push_back(ch);
    }
    return result;
}