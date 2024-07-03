#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadnum,const std::string threadtype):stop_(false),threadtype_(threadtype)
{
    // 启动threadnum个线程，每个线程将阻塞在条件变量上
    for (size_t ii = 0; ii < threadnum; ii++)
    {
        // 用lambda函数创建线程
        threads_.emplace_back([this]() -> void
        {
            // 线程会一直运行函数体的内容
            // 显示线程的id
            printf("create %s thread(%ld).\n",threadtype_.c_str(),syscall(SYS_gettid));

            while (stop_ == false)
            {
                std::function<void()> task;
                { // 锁的作用域开始
                    
                    std::unique_lock<std::mutex> lock(this->mutex_);

                    // 等待生产者的条件变量  如果任务队列中没有任务，会阻塞在此处
                    this->condition_.wait(lock, [this]
                    {
                        return ((this->stop_ == true) || (this->taskqueue_.empty() == false));
                    });

                    // 在线程池停止之前，如果队列中还有任务，执行完再退出
                    if((this->stop_ == true) && (this->taskqueue_.empty() == true)) return;
                    
                    // 右值引用将所有权转移给task
                    task = std::move(this->taskqueue_.front());
                    this->taskqueue_.pop();
                } // 锁的作用域结束
                printf("%s(%ld) execute task.\n",threadtype_.c_str(),syscall(SYS_gettid));
                // 执行任务
                task(); 
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    stop_ = true;
    // 唤醒全部的线程
    condition_.notify_all();
    // 等待全部线程执行完任务后退出，释放线程资源
    for (std::thread &th : threads_)
        th.join();
}

// 将任务添加到队列中
void ThreadPool::addtask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }
    // 唤醒一个线程
    condition_.notify_one();
}
