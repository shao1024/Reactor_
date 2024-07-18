#include "Acceptor.h"
#include "Connection.h"

Acceptor::Acceptor(EventLoop* loop,const std::string &ip,uint16_t port)
            :loop_(loop),servsock_(createnonblocking()),acceptchannel_(loop_,servsock_.fd())
{
     // 创建非阻塞的服务端套接字
    //servsock_ = new Socket(createnonblocking());
    // 设置网络通信用的协议
    InetAddress servaddr(ip,port);
    // 设置网络中的端口复用、延时等属性的设置
    servsock_.setreuseaddr(true);
    servsock_.setreuseport(true);
    servsock_.settcpnodelay(true);
    servsock_.setkeepalive(true);
    // 绑定ip与端口号
    servsock_.bind(servaddr);
    // 服务端开始监听
    servsock_.listen();

    //acceptchannel_ = new Channel(loop_,servsock_->fd());
    acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection,this));
    acceptchannel_.enablereading(); // 让epoll_wait()监视servchannel的读事件。
}

Acceptor::~Acceptor()
{
    //delete servsock_;
    //delete acceptchannel_;
}


// 处理新客户端连接请求
void Acceptor::newconnection()
{
    InetAddress clientaddr;
    // 连接客户端，同时将连上的客户端套接字设置为非阻塞
    //Socket *clientsocket = new Socket(servsock_.accept(clientaddr));
    std::unique_ptr<Socket> clientsocket(new Socket(servsock_.accept(clientaddr)));
    clientsocket->setipport(clientaddr.ip(),clientaddr.port());
    //printf ("accept client(fd=%d,ip=%s,port=%d) ok.\n",clientsocket->fd(),clientaddr.ip(),clientaddr.port());
    
    //Connection *conn=new Connection(loop_,clientsocket);
    // 使用的是上一级类TcpServer中的函数作为回调函数，将connection构建到TcpServer中便于管理
    newconnectioncb_(std::move(clientsocket));
}

// 设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置
void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn)
{
    newconnectioncb_ = fn;
}