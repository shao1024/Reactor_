#include "Socket.h"
#include <unistd.h>

// 创建一个非阻塞的socket,返回套接字；可配合Socket的构造函数使用
int createnonblocking()
{
    // 创建服务端用于监听的listenfd
    int listenfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
    if(listenfd < 0){
        printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return listenfd;
}

Socket::Socket(int fd):fd_(fd)
{


}

Socket::~Socket()
{
    ::close(fd_);

}


// 返回套接字
int Socket::fd() const
{
    return fd_;
}

// 返回port_成员
uint16_t Socket::port() const
{
    return port_;
}

// 返回ip_成员
std::string Socket::ip() const
{
    return ip_;
}



// 设置SO_REUSEADDR选项，允许重复使用本地地址（端口）
void Socket::setreuseaddr(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)) );
}


// 设置SO_REUSEPORT选项，允许多个套接字绑定到同一个端口上
void Socket::setreuseport(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)) );
}  


// 设置TCP_NODELAY选项，禁用Nagle算法  
void Socket::settcpnodelay(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)) );
}


// 设置SO_KEEPALIVE选项，用于检测对端是否仍然在线，特别是在长连接中避免“半开连接”的问题。
void Socket::setkeepalive(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)) );
}       


// 绑定端口号和ip
void Socket::bind(const InetAddress& servaddr)
{
    if(::bind(fd_,(sockaddr *)&servaddr,sizeof(servaddr)) < 0){
        perror("bind");close(fd_);
        exit(-1) ;
    }
    setipport(servaddr.ip(),servaddr.port());
}


// 服务端开始监听,nn表示缓存队列，监听到还没处理的放入此处
void Socket::listen(int nn)
{
    if(::listen(fd_,nn) != 0){
        perror("listen()");close(fd_);
        exit(-1) ;
    }

}

// 设置ip_和port_成员
void Socket::setipport(const std::string &ip,uint16_t port)
{
    ip_ = ip;
    port_ = port;
}


// 受理客户端连接，并将客户端的套接字设置为非阻塞
int Socket::accept(InetAddress& clientaddr)
{
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    // accept4与accept的区别是accept4允许在连接被接受时直接设置套接字描述符（是否阻塞）的标志
    int clientfd = accept4(fd_,(sockaddr *)&peeraddr,&len,SOCK_NONBLOCK);
    clientaddr.setaddr(peeraddr);

    return clientfd;
}