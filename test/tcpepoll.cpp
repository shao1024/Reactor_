#include <iostream>
#include <sys/epoll.h>
#include <netinet/tcp.h>  // TCP_NODELAY需要包含这个头文件。
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>    
using namespace std;

/**
 * 设置文件描述符为非阻塞模式
 *
 * @param fd 文件描述符。这是一个整数，标识已打开的文件或者套接字。
 *
 * 函数说明：
 * 此函数将指定的文件描述符fd设置为非阻塞模式。在非阻塞模式下，对fd进行读取或写入操作时，
 * 如果没有数据可读或没有足够的缓冲区空间可写，操作会立即返回一个错误（EWOULDBLOCK或EAGAIN），
 * 而不是像阻塞模式那样等待数据变得可用或缓冲区有空间。
 *
  */
void setnonblocking(int fd){
 /* 实现方式：
 * 首先，通过fcntl(fd, F_GETFL)获取fd当前的文件状态标志。
 * 然后，使用按位或运算符(|)将O_NONBLOCK标志添加到当前的文件状态标志中。
 * 最后，通过fcntl(fd, F_SETFL, new_flags)将修改后的标志设置回文件描述符，实现非阻塞模式的设置。
 */
fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
}

class Channel
{
private:
    int fd_;
    bool islisten_;
public:
    Channel(int fd,bool islisten = false):fd_(fd),islisten_(islisten) {}
    int fd() {return fd_;}
    bool islisten() {return islisten_;}
    ~Channel();
};


int main(int argc,char* argv[]){
    if(argc != 3){
        cout<<"usage: ./tcpepoll ip port\n"<<"example: ./tcpepoll 192.168.150.128 5085\n\n"<<endl;
        return -1;
    }

    //1. 申请套接字
    int listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(listenfd < 0){
        perror("socket");
        return -1;
    }

    //2. 设置连接属性
    int opt = 1;
    //必须的 在服务器重启后立即绑定到之前使用过的端口。SOL_SOCKET指定了选项级别为套接字，SO_REUSEADDR是具体的选项，它允许重复使用本地地址（端口）
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)) );
    //必须的 此调用设置了TCP_NODELAY选项，它禁用了Nagle算法。
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)) );
    //可选的 允许多个套接字绑定到同一个端口上，操作系统会负责分配连接到这些端口的请求给不同的套接字。
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)) );
    // 可选的 开启TCP Keepalive功能，用于检测对端是否仍然在线，特别是在长连接中避免“半开连接”的问题。
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)) );
    //将监听设置为非阻塞（一般与边缘触发配合使用）
    setnonblocking(listenfd);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    //3.绑定端口号和ip
    if(bind(listenfd,(sockaddr *)&servaddr,sizeof(servaddr)) < 0){
        perror("bind");close(listenfd);
        return -1;
    }

    //4.服务端开始监听
    if(listen(listenfd,128) != 0){
        perror("listen()");close(listenfd);
        return -1;
    }

    //5.创建epoll句柄
    int epollfd = epoll_create(1);
    // 返回的提示消息，套接字序号以及是否为监听套接字
    Channel *servchannel = new Channel(listenfd,true);
    // 声明epoll事件的数据结构。
    struct epoll_event ev;
    ev.data.ptr = servchannel; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = EPOLLIN;  // 让epoll监视listenfd的读事件，采用水平触发(缺省)。
    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev); // 把需要监视的listenfd和它的事件加入epollfd中。

    //6. 事件循环
    // epoll句柄中发生事件后会通过epoll_wait()返回并存储到该数组中
    struct epoll_event evs[10];
    while (true)
    {
        // 超时选项设置为-1，表示不启用超时
        int infds = epoll_wait(epollfd,evs,10,-1);
        if(infds <0){
            perror("epoll_wait");break;
        }
        if(infds == 0){
            perror("time out");break;
        }

        // 如果infds>0，表示有事件发生的fd的数量。
        for (int i = 0; i < infds; i++)
        {
            //判断是否为用于监听的套接字有事件
            Channel *ch=(Channel *)evs[i].data.ptr;
            // 有客户端连上来
            if (ch->islisten() == true)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd,(sockaddr *)&clientaddr,&len);
                setnonblocking(clientfd);         // 客户端连接的fd必须设置为非阻塞的。
                cout<<"accept client(fd="<<clientfd<<",ip="<<inet_ntoa(clientaddr.sin_addr)<<",port="<<ntohs(clientaddr.sin_port)<<"),OK"<<endl;

                //设置新脸上来的客户端会使用哪种触发模式
                Channel *clientchannel = new Channel(clientfd);
                ev.data.ptr = clientchannel;
                ev.events = EPOLLIN|EPOLLET;   //设置读事件，采用边缘触发
                epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev);
                
            }
            else{//有消息发过来
                if(evs[i].events & EPOLLRDHUP)// 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
                {
                    cout<<"1client(eventfd="<<ch->fd()<<") didconnect"<<endl;
                    close(ch->fd());
                }
                else if(evs[i].events & (EPOLLIN|EPOLLPRI)) // 有数据可读
                {
                    char buffer[1024];
                    // 因为使用了非阻塞io，所以要一次性全部读完缓存区内容
                    while (true)
                    {
                        bzero(&buffer,sizeof(buffer));
                        ssize_t nread = read(ch->fd(),buffer,sizeof(buffer));
                        if(nread >0)// 成功的读取到了数据。
                        {
                            printf("recv(eventfd=%d):%s\n",ch->fd(),buffer);
                            send(ch->fd(),buffer,strlen(buffer),0);
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
                            cout<<"1client(eventfd="<<ch->fd()<<") didconnect"<<endl;
                            close(ch->fd());
                            break;
                        } 
                    }
                }
                else if(evs[i].events & EPOLLOUT)// 有数据需要写，暂时没有代码
                {

                }
                else// 其它事件，都视为错误，关闭套接字
                {
                    printf("3client(eventfd=%d) error.\n",ch->fd());
                    close(ch->fd());   
                }
            }
        }
    }
    return 0;
}
