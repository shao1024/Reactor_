#include "EventLoop.h"

// 创建一个定时器文件描述符，并设置了一个初始超时时间
int createtimerfd(int sec=30)
{
    // CLOCK_MONOTONIC：使用单调递增的时钟源 
    // TFD_CLOEXEC：在执行新的程序（通过 exec 系列函数）时，文件描述符将自动关闭，避免被新程序继承。
    // TFD_NONBLOCK：文件描述符处于非阻塞模式，读写操作不会阻塞等待数据或空间。
    int tfd =  timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
    struct itimerspec timeout;
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd,0,&timeout,0);
    return tfd;
}


EventLoop::EventLoop(bool mainloop,int timetv1,int timeout):ep_(new Epoll),
                mainloop_(mainloop),timetv1_(timetv1),timeout_(timeout),stop_(false),
                wakeupfd_(eventfd(0,EFD_NONBLOCK)),wakeupchannel_(new Channel(this,wakeupfd_)),
                timerfd_(createtimerfd(timeout_)),timerchannel_(new Channel(this,timerfd_))
{
    wakeupchannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakeupchannel_->enablereading();

    timerchannel_->setreadcallback(std::bind(&EventLoop::handletimer,this));
    timerchannel_->enablereading();

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

void EventLoop::stop()
{
    stop_ = true;
    wakeup();
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
    printf("handlewakeup() thread id is %ld.\n",syscall(SYS_gettid));

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


// 闹钟响时执行的函数
void EventLoop::handletimer()
{
    // 重新计时
    struct itimerspec timeout;
    memset(&timeout,0,sizeof(itimerspec));
    timeout.it_value.tv_sec = timetv1_;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_,0,&timeout,0);

    if(mainloop_)
    {
        // printf("主事件循环的闹钟时间到了。\n");
    }
    else
    {
        // printf("从事件循环的闹钟时间到了。\n"); 
        // 注：如果直接使用  for(auto ch : conns_)的方式访问哈希表可能出现问题
        time_t now = time(0);
        for (auto it=conns_.begin(); it != conns_.end();)
        {
            if(it->second->timeout(now,timeout_))
            {
                timercallback_(it->first);
                std::lock_guard<std::mutex> gd(mutex_);
                it = conns_.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
}

// 把Connection对象保存在conns_中
void EventLoop::newconnection(spConnection conn)
{
    // 可能在主线程中添加conn  可能在子线程中触发闹钟函数回调TcpServer::sendcomplete删除conn
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_[conn->fd()] = conn;
}

// 将被设置为TcpServer::removeconn()
void EventLoop::settimercallback(std::function<void(int)> fn)
{
    timercallback_ = fn;
}
