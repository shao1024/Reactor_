#include "EventLoop.h"

EventLoop::EventLoop():ep_(new Epoll)
{

}

EventLoop::~EventLoop()
{
    delete ep_;
}


// 运行事件循环。
void EventLoop::run()
{
    // 事件循环
    while (true)
    {
        // 等待监视的fd有事件发生。
        std::vector<Channel *> channels = ep_->loop();
        for (auto &ch:channels)
        {
            // 处理epoll_wait()返回的事件。
            ch->handleevent();
        }
    }

}
    
// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void EventLoop::updatechannel(Channel *ch)
{
    ep_->updatechannel(ch);
}