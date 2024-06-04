#include "Channel.h"
#include <unistd.h>
#include <string.h>
Channel::Channel(Epoll* ep,int fd,bool islisten):ep_(ep),fd_(fd),islisten_(islisten)
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


// 采用边缘触发。
void Channel::useet()
{
    events_ = events_|EPOLLET;
}


// 设置epoll_wait()监视fd_的读事件。
void Channel::enablereading()
{
    events_ |= EPOLLIN;
    ep_->updatechannel(this);
}


// 把inepoll_成员的值设置为true。
void Channel::setinepoll()
{
    inepoll_ = true;
}


// 设置revents_成员，即记录当前fd发生的事件
void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}


// 事件处理函数，epoll_wait()返回的时候，执行它，对读事件与写事件等进行操作
void Channel::handleevent(Socket *servsock)
{
    if(revents_ & EPOLLRDHUP)// 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        printf("client(eventfd=%d) disconnected.\n",fd_);
        ::close(fd_);
    }
    else if(revents_ & (EPOLLIN|EPOLLPRI)) // 有数据可读
    {
        if (islisten_ == true)
        {
            InetAddress clientaddr;
            // 连接客户端，同时将连上的客户端套接字设置为非阻塞
            Socket *clientsocket = new Socket(servsock->accept(clientaddr));
            printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientsocket->fd(),clientaddr.ip(),clientaddr.port());
    
            // 为新客户端连接准备读事件，并添加到epoll中。
            Channel *clientchannel = new Channel(ep_,clientsocket->fd(),false);
            clientchannel->useet(); // 客户端连上来的fd采用边缘触发。
            clientchannel->enablereading();// 让epoll_wait()监视clientchannel的读事件。
            
        }
        else
        {
            char buffer[1024];
        // 因为使用了非阻塞io，所以要一次性全部读完缓存区内容
            while (true)
            {
                bzero(&buffer,sizeof(buffer));
                ssize_t nread = read(fd_,buffer,sizeof(buffer));
                if(nread >0)// 成功的读取到了数据。
                {
                    printf("recv(eventfd=%d):%s\n",fd_,buffer);
                    send(fd_,buffer,strlen(buffer),0);
                }
                else if(nread == -1 && errno == EINTR)// 读取数据的时候被信号中断，继续读取。
                {
                    continue;
                }
                else if(nread == -1 && ((errno == EAGAIN)||(errno == EWOULDBLOCK)))// 全部的数据已读取完毕
                {
                    break;
                }
                else if(nread == 0)// 客户端连接已断开
                {
                    printf("client(eventfd=%d) disconnected.\n",fd_);
                    close(fd_);
                    break;
                } 
            }
        }

    }   
    else if(revents_ & EPOLLOUT)// 有数据需要写，暂时没有代码
    {

    }
    else// 其它事件，都视为错误，关闭套接字
    {
        printf("3client(eventfd=%d) error.\n",fd_);
        close(fd_);   
    }
}

