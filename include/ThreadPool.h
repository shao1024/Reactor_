#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include <unistd.h>
#include <sys/syscall.h>

#include <vector>
#include <string>
#include <queue>
#include <functional>

#include <atomic>

class ThreadPool
{
private:
    // 线程池中的线程
    std::vector<std::thread> threads_;
    // 任务队列
    std::queue<std::function<void()>> taskqueue_;
    // 任务队列同步的互斥锁
    std::mutex mutex_;
    // 任务队列同步的条件变量
    std::condition_variable condition_;
    // 在析构函数中，把stop_的值设置为true，全部的线程将退出
    std::atomic_bool stop_;
    // 线程的类别 "IO"用于connection通信  "WORK"用于计算业务
    const std::string threadtype_;


public:
    // 在构造函数中启动threadnum个线程
    ThreadPool(size_t threadnum, const std::string threadtype);
    // 在析构函数中停止线程
    ~ThreadPool();

    // 获取线程池中线程的个数
    size_t size();

    // 将任务添加到队列中
    void addtask(std::function<void()> task);

    // 停止线程
    void stop();
};



