#pragma once
#include "Epoll.h"
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <map>
#include <atomic>
#include "Connection.h"
#include <sys/timerfd.h>
#include <string.h>

class Channel;
class Epoll;
class Connection;
using spConnection=std::shared_ptr<Connection>;

class EventLoop
{
private:
    // 闹钟时间间隔，单位：秒
    int timetv1_;
    // Connection对象超时的时间，单位：秒
    int timeout_;
    // 定时器的fd
    int timerfd_;
    // 定时器的Channel
    std::unique_ptr<Channel> timerchannel_;
    // true-是主事件循环，false-是从事件循环
    bool mainloop_;
    // 保护conns_的互斥锁
    std::mutex mmutex_;
    // 存放运行在该事件循环上全部的Connection对象
    std::map<int,spConnection> conns_;
    // 删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeconn()
    std::function<void(int)> timercallback_;
    // 初始值为false，如果设置为true，表示停止事件循环
    std::atomic_bool stop_;


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
    // 在构造函数中创建Epoll对象ep_
    EventLoop(bool mainloop,int timetv1=30,int timeout=80);
    ~EventLoop();

    // 运行事件循环。
    void run();
    // 停止事件循环
    void stop();
    
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

    // 闹钟响时执行的函数
    void handletimer();
    // 把Connection对象保存在conns_中
    void newconnection(spConnection conn);
    // 将被设置为TcpServer::removeconn()
    void settimercallback(std::function<void(int)> fn);

};
