#include "Connection.h"
#include <string.h>
#include <unistd.h>

Connection::Connection(EventLoop* loop,std::unique_ptr<Socket> clientsock)
        :loop_(loop),clientsock_(std::move(clientsock)),disconnect_(false),clientchannel_(new Channel(loop_,clientsock_->fd()))
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    //clientchannel_ = new Channel(loop_,clientsock_->fd());
    // 新生成的套接字均为接受消息的功能，使用onmessage作为回调函数，在执行onmessage函数时，隐含要求该类的指针，所以将当前channel作为参数传进去
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage,this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback,this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback,this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback,this));
    clientchannel_->useet(); // 客户端连上来的fd采用边缘触发。
    clientchannel_->enablereading();// 让epoll_wait()监视clientchannel的读事件。
}

Connection::~Connection()
{
    //delete clientsock_;
    //delete clientchannel_;

}

// 返回客户端的fd
int Connection::fd() const
{
    return clientsock_->fd();
}

// 返回客户端的ip
std::string Connection::ip() const
{
    return clientsock_->ip();
}

// 返回客户端的port
uint16_t Connection::port() const
{
    return clientsock_->port();
}


// TCP连接关闭（断开）的回调函数，供Channel回调
void Connection::closecallback()
{
    disconnect_ = true;
    clientchannel_->remove();
    // 使用上层Tcpserver的关闭回调函数
    closecallback_(shared_from_this());
}


// TCP连接错误的回调函数，供Channel回调
void Connection::errorcallback()
{
    disconnect_ = true;
    clientchannel_->remove();
    // 使用上层Tcpserver的错误回调函数
    errorcallback_(shared_from_this());
}

// 设置关闭fd_的回调函数。
void Connection::setclosecallback(std::function<void(spConnection)> fn)
{
    closecallback_ = fn;
}

// 设置fd_发生了错误的回调函数。
void Connection::seterrorcallback(std::function<void(spConnection)> fn)
{
    errorcallback_ = fn;
}

// 设置消息的回调函数
void Connection::setonmessagecallback(std::function<void(spConnection,std::string&)> fn)
{
    onmessagecallback_ = fn;
}

// 发送数据完成后的回调函数
void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)
{
    sendcompletecallback_ = fn;
}

// 处理对端发送过来的消息,给Channel回调
void Connection::onmessage()
{
    char buffer[1024];
    // 因为使用了非阻塞io，所以要一次性全部读完缓存区内容
    while (true)
    {
        // 清空buffer
        bzero(&buffer,sizeof(buffer));
        // 读取接收缓存区的数据
        ssize_t nread = read(fd(),buffer,sizeof(buffer));
        if(nread >0)// 成功的读取到了数据。
        {
            //printf("recv(eventfd=%d):%s\n",fd(),buffer);
            // send(fd(),buffer,strlen(buffer),0);
            inputbuffer_.append(buffer,nread);
        }
        else if(nread == -1 && errno == EINTR)// 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if(nread == -1 && ((errno == EAGAIN)||(errno == EWOULDBLOCK)))// 全部的数据已读取完毕
        {
            // printf("recv(eventfd=%d):%s\n",fd(),inputbuffer_.data());
            // // 在这里，将经过若干步骤的运算

            // // 运算后的结果已存放在outputbuffer_中
            // outputbuffer_ = inputbuffer_;
            // inputbuffer_.clear();
            // send(fd(),outputbuffer_.data(),outputbuffer_.size(),0);

            // 从接收缓冲区中拆分出客户端的请求消息,直到所有的消息都已经拆分完成
            while (true)
            {
                // 存储报文长度
                int len;
                // 提取前四个字节的数据
                memcpy(&len,inputbuffer_.data(),4);
                // 判断报文是否完整；大于或等于长度代表接收缓存区有一条或一条多的数据
                if (inputbuffer_.size() < len + 4) break;

                // 把消息分离出来
                std::string message(inputbuffer_.data()+4,len);
                // 从inputbuffer中删除刚才已获取的报文
                inputbuffer_.erase(0,len+4);

                printf("message (eventfd=%d):%s\n",fd(),message.c_str());
                lasttime_ = Timestamp::now();
                // 回调TcpServer::onmessage()处理客户端的请求消息
                onmessagecallback_(shared_from_this(),message);
                
            }
            break;
        }
        else if(nread == 0)// 客户端连接已断开
        {
            // printf("client(eventfd=%d) disconnected.\n",fd_);
            // close(fd_);
            closecallback();
            break;
        } 
    }
}


// 发送数据,不管在任何线程中，都是调用此函数发送数据。
void Connection::send(const char* data,size_t size)
{
    if (disconnect_==true) {  printf("客户端连接已断开了，send()直接返回。\n"); return;}

    std::shared_ptr<std::string> message(new std::string(data));
    // 如果当前线程是IO线程，直接调用send()
    if(loop_->isinloopthread())
    {
        printf("send() 在事件循环的线程中。\n");
        sendinloop(message);
    }
    else
    {
        printf("send() 不在事件循环的线程中。\n");
        //std::cout<<size<<std::endl;
        loop_->queueinloop(std::bind(&Connection::sendinloop,this,message));
    }
    
}

// 发送数据，如果当前线程是IO线程，直接进行调用；如果是工作线程，将此函数传去IO线程中执行
void Connection::sendinloop(std::shared_ptr<std::string> data)

{
    outputbuffer_.appendwithhead(data->data(),data->size()); // 把需要发送的数据保存到Connection的发送缓冲区中
    clientchannel_->enablewriting(); // 注册写事件
}

// 处理写事件的回调函数，供Channel回调
void Connection::writecallback()
{
    // 将outputbuffer_中的数据发布出去
    int writen = ::send(fd(),outputbuffer_.data(),outputbuffer_.size(),0);
    if(writen>0) outputbuffer_.erase(0,writen);

    // 如果发送缓存区中没有数据，代表数据发送完成，停止关注写事件
    if(outputbuffer_.size() == 0) 
    {
        clientchannel_->disablewriting();
        sendcompletecallback_(shared_from_this());
    }
}

// 判断TCP连接是否超时（空闲太久）val为要求的最大间隔时长
bool Connection::timeout(time_t now,int val)
{
    return now - lasttime_.toint() > val;
}



