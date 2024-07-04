#include "Channel.h"
#include "Connection.h"
#include <unistd.h>
#include <string.h>

Channel::Channel(const std::unique_ptr<EventLoop>& loop,int fd):loop_(loop),fd_(fd)
{

}

Channel::~Channel()
{
    // 在析构函数中，不要销毁ep_，也不能关闭fd_，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已。
}


// 返回fd_成员。
int Channel::fd()
{
    return fd_;
}


// 返回events_成员
uint32_t Channel::events()
{
    return events_;
}


// 返回revents_成员
uint32_t Channel::revents()
{
    return revents_;
}


// 返回inepoll_成员
bool Channel::inepoll()
{
    return inepoll_;
}

// 把inepoll_成员的值设置为true。
void Channel::setinepoll(bool inepoll)
{
    inepoll_ = inepoll;
}


// 采用边缘触发。
void Channel::useet()
{
    events_ = events_|EPOLLET;
}


// 设置epoll_wait()监视fd_的读事件。
void Channel::enablereading()
{
    events_ |= EPOLLIN;
    loop_->updatechannel(this);
}

// 取消读事件
void Channel::disablereading()
{
    events_ &= ~EPOLLIN;
    loop_->updatechannel(this);

}

// 注册写事件
void Channel::enablewriting()
{
    events_ |= EPOLLOUT;
    loop_->updatechannel(this);

}

// 取消写事件
void Channel::disablewriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updatechannel(this);
}

// 取消全部的事件
void Channel::disableall()
{
    events_ = 0;
    loop_->updatechannel(this);

} 

// 从事件循环中删除Channel
void Channel::remove()
{
    disableall();
    loop_->removechannel(this);

}



// 设置revents_成员，即记录当前fd发生的事件
void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}


// 事件处理函数，epoll_wait()返回的时候，执行它，对读事件与写事件等进行操作
void Channel::handleevent()
{
    if(revents_ & EPOLLRDHUP)// 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        // 回调Connection::closecallback()
        closecallback_();
    }
    else if(revents_ & (EPOLLIN|EPOLLPRI)) // 有数据可读
    {
        // 如果是acceptchannel，将回调Acceptor::newconnection()，如果是clientchannel，将回调Connection::onmessage()
        readcallback_();
    }   
    else if(revents_ & EPOLLOUT)// 有数据需要写，暂时没有代码
    {
         // 回调Connection::writecallback()
        writecallback_();
    }
    else// 其它事件，都视为错误，关闭套接字
    {
        // 回调Connection::errorcallback() 
        errorcallback_();
    }
}

// 设置fd_读事件的回调函数
void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_ = fn;
}

// 设置关闭fd_的回调函数
void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_ = fn;
}

// 设置fd_发生了错误的回调函数
void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_ = fn;
}


// 设置写事件的回调函数
void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_ = fn;
}
