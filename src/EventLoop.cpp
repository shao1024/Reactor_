#include "EventLoop.h"

EventLoop::EventLoop()
                :ep_(new Epoll),wakeupfd_(eventfd(0,EFD_NONBLOCK)),wakeupchannel_(new Channel(this,wakeupfd_))
{
    wakeupchannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakeupchannel_->enablereading();
}

EventLoop::~EventLoop()
{
    //delete ep_;
}


// 运行事件循环。
void EventLoop::run()
{
    // 获取事件循环所在线程的id
    threadid_=syscall(SYS_gettid);    
    // 事件循环
    while (true)
    {
        // 等待监视的fd有事件发生。
        std::vector<Channel *> channels = ep_->loop(10*1000);

        // 如果channels为空，表示超时，回调TcpServer::epolltimeout()
        if (channels.size() == 0)
        {
            epolltimeoutcallback_(this);
        }
        else
        {
            for (auto &ch:channels)
            {
                // 处理epoll_wait()返回的事件。
                ch->handleevent();
            }
        }
    }

}
    
// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void EventLoop::updatechannel(Channel *ch)
{
    ep_->updatechannel(ch);
}

// 从黑树上删除channel
void EventLoop::removechannel(Channel *ch)
{
    ep_->removechannel(ch);
}

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop*)> fn)
{
    epolltimeoutcallback_ = fn;
}

// 判断当前线程是否为事件循环线程
bool EventLoop::isinloopthread()
{
    return threadid_ == syscall(SYS_gettid);
}

// 把任务添加到队列中
void EventLoop::queueinloop(std::function<void()> fn)
{
    {
        // 给任务队列加锁
        std::lock_guard<std::mutex> gd(mutex_);
        // 任务入队
        taskqueue_.push(fn);
    }
    // 唤醒事件循环
    wakeup();
}

// 用eventfd唤醒事件循环线程
void EventLoop::wakeup()
{
    uint64_t val = 1;
    write(wakeupfd_,&val,sizeof(val));
}

// 事件循环线程被eventfd唤醒后执行的函数
void EventLoop::handlewakeup()
{
    printf("handlewakeup() thread id is %d.\n",syscall(SYS_gettid));

    uint64_t val;
    // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发
    read(wakeupfd_,&val,sizeof(val));

    std::function<void()> fn;
    std::lock_guard<std::mutex> gd(mutex_);

    // 执行队列中全部的发送任务
    while (taskqueue_.size() > 0)
    {
        // 出队一个元素
        fn = std::move(taskqueue_.front());
        taskqueue_.pop();
        // 执行任务
        fn();
    }
}
