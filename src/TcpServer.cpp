#include "TcpServer.h"
#include <unistd.h>
#include <iostream>

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum)
            :threadnum_(threadnum),mainloop_(new EventLoop),acceptor_(mainloop_.get(),ip,port),threadpool_(threadnum_,"IO")
{
    // 创建主事件循环
    //mainloop_ = new EventLoop;
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));

    // 将acceptor放入主事件循环中运行
    //acceptor_ = new Acceptor(mainloop_,ip,port);
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
    
    // 创建线程池
    //threadpool_ = new ThreadPool(threadnum_,"IO"); 
    // 创建从事件循环并将每个从事件循环的run函数添加到任务队列
    //std::cout<<threadnum_<<std::endl;
    for (int ii=0; ii<threadnum_; ii++){
        //subloops_.push_back(std::move(std::make_unique<EventLoop>() ));
        subloops_.emplace_back(new EventLoop);
        // 设置timeout超时的回调函数
        subloops_[ii]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));
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
    conns_[conn->fd()] = conn;

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
    
    //printf("client(eventfd=%d) disconnected.\n",conn->fd());
    //::close(conn->fd());
    conns_.erase(conn->fd());
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
    
    //printf("client(eventfd=%d) error.\n",conn->fd());
    //::close(conn->fd());
    conns_.erase(conn->fd());
    // 会一层一层最后关闭对应的套接字
    //delete conn;
}


// 处理客户端的请求报文，在Connection类中回调此函数
void TcpServer::onmessage(spConnection conn,std::string& message)
{
    // 处理报文的一些业务
    /*
    message = "reply:" + message;
    // 计算报文大小
    int len = message.size();
    // 添加报文头部4字节，表示报文长度
    std::string tmpbuf((char*)&len,4);
    // 添加报文内容
    tmpbuf.append(message);
    //std::cout<< "here"<<std::endl;
    // 发送报文
    conn->send(tmpbuf.data(),tmpbuf.size());
    */
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
