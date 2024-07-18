#pragma once
#include "Epoll.h"
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>

class Channel;
class Epoll;

class EventLoop
{
private:
    // 每个事件循环只有一个Epoll
    std::unique_ptr<Epoll> ep_;
    // 每个事件循环都有一个线程id
    pid_t threadid_;
    // 用于唤醒事件循环线程的eventfd
    int wakeupfd_;
    // eventfd的Channel
    std::unique_ptr<Channel> wakeupchannel_;
    // 任务队列同步的互斥锁
    std::mutex mutex_;

    // epoll_wait()超时的回调函数 回调TcpServer::epolltimeout
    std::function<void(EventLoop*)> epolltimeoutcallback_;
    // 事件循环线程被eventfd唤醒后执行的任务队列
    std::queue<std::function<void()>> taskqueue_;

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

    // 判断当前线程是否为事件循环线程
    bool isinloopthread();
    // 把任务添加到队列中
    void queueinloop(std::function<void()> fn);
    // 用eventfd唤醒事件循环线程
    void wakeup();
    // 事件循环线程被eventfd唤醒后执行的函数
    void handlewakeup();

};
