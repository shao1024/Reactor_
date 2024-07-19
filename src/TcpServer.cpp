#include "TcpServer.h"
#include <unistd.h>
#include <iostream>
#include "Timestamp.h"

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum)
            :threadnum_(threadnum),mainloop_(new EventLoop(true)),
            acceptor_(mainloop_.get(),ip,port),threadpool_(threadnum_,"IO")
{
    // 设置epoll_wait()超时的回调函数
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));

    // 设置处理新客户端连接请求的回调函数
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
    
    // 创建从事件循环并将每个从事件循环的run函数添加到任务队列
    //std::cout<<threadnum_<<std::endl;
    for (int ii=0; ii<threadnum_; ii++){
        subloops_.emplace_back(new EventLoop(false,5,10));
        // 设置timeout超时的回调函数
        subloops_[ii]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));
        // 设置清理空闲TCP连接的回调函数
        subloops_[ii]->settimercallback(std::bind(&TcpServer::removeconn,this,std::placeholders::_1));
        // 在线程池中运行从事件循环  
        threadpool_.addtask(std::bind(&EventLoop::run,subloops_[ii].get()));
        //std::cout<<"hello"<<std::endl;
    }
}

TcpServer::~TcpServer()
{
    //delete mainloop_;
    //delete acceptor_;
    // 释放所有的Connection对象
    // for (auto &aa:conns_)
    // {
    //     delete aa.second;
    // }

    // // 释放从事件循环
    // for(auto &aa:subloops_)
    // {
    //     delete aa;
    // }
    
    // delete threadpool_;
}

void TcpServer::start()
{
    mainloop_->run();

}

// 停止IO线程和事件循环
void TcpServer::stop()
{
    // 停止主事件循环
    mainloop_->stop();
    printf("主事件循环已停止。\n");

    // 停止从事件循环
    for (int i = 0; i < threadnum_; i++)
    {
        subloops_[i]->stop();
    }
    printf("从事件循环已停止。\n");

    // 停止IO线程
    threadpool_.stop();
    printf("IO线程池停止。\n");
}

void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
    // 把新建的conn分配给从事件循环
    spConnection conn(new Connection(subloops_[clientsock->fd() % threadnum_].get(),std::move(clientsock)));
    // 设置Connection的回调函数，
    conn->setclosecallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage,this,std::placeholders::_1,std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete,this,std::placeholders::_1));

    //printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
    // 把conn存放map容器中
    {
        // 可能在主线程中添加conn  可能在子线程中触发闹钟函数回调TcpServer::sendcomplete删除conn
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_[conn->fd()] = conn;
    }

    // 把conn存放到EventLoop的map容器中
    subloops_[conn->fd() % threadnum_]->newconnection(conn);
    // 回调EchoServer::HandleNewConnection()
    if (newconnectioncb_) newconnectioncb_(conn);
}

// 关闭客户端的连接，在Connection类中回调此函数
void TcpServer::closeconnection(spConnection conn)
{
    if (closeconnectioncb_)
    {
       closeconnectioncb_(conn);
    }

    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(conn->fd());
    }
    
    // 会一层一层最后关闭对应的套接字
    //delete conn;
}

// 客户端的连接错误，在Connection类中回调此函数
void TcpServer::errorconnection(spConnection conn)
{
    if (errorconnectioncb_)
    {
        errorconnectioncb_(conn);
    }
    
    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(conn->fd());
    }
}


// 处理客户端的请求报文，在Connection类中回调此函数
void TcpServer::onmessage(spConnection conn,std::string& message)
{
    if (onmessagecb_)
    {
        onmessagecb_(conn,message);
    }

}

// 数据发送完成后，在Connection类中回调此函数
void TcpServer::sendcomplete(spConnection conn)
{
    if (sendcompletecb_)
    {
        sendcompletecb_(conn);
    }
    
}

// epoll_wait()超时，在EventLoop类中回调此函数
void TcpServer::epolltimeout(EventLoop* loop)
{
    if (epolltimeoutcb_)
    {
        epolltimeoutcb_(loop);
    }

}

// 删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数
void TcpServer::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> gd(mmutex_);
        // 从map中删除conn
        conns_.erase(fd);
    }

}




void TcpServer::setnewconnectioncb(std::function<void(spConnection)> fn)
{
    newconnectioncb_ = fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(spConnection)> fn)
{
    closeconnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(spConnection)> fn)
{
    errorconnectioncb_ = fn;
}

// 
void TcpServer::setonmessagecb(std::function<void(spConnection,std::string &message)>  fn)
{
    onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(std::function<void(spConnection)> fn)
{
    sendcompletecb_ = fn;
}

void TcpServer::setepolltimeoutcb(std::function<void(EventLoop*)> fn)
{
    epolltimeoutcb_ = fn;
}
