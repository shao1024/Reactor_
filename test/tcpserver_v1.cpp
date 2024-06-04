#include "InetAddress.h"
#include "Epoll.h"
#include "Socket.h"
#include "Channel.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("usage: ./tcpepoll ip port\n"); 
        printf("example: ./bin/tcpserver_v1 172.25.104.182 5005\n\n"); 
        return -1;
    }

    // 创建非阻塞的服务端套接字
    Socket servsock(createnonblocking());
    // 设置网络通信用的协议
    InetAddress servaddr(argv[1],atoi(argv[2]));
    // 设置网络中的端口复用、延时等属性的设置
    servsock.setreuseaddr(true);
    servsock.setreuseport(true);
    servsock.settcpnodelay(true);
    servsock.setkeepalive(true);
    // 绑定ip与端口号
    servsock.bind(servaddr);
    // 服务端开始监听
    servsock.listen();

    Epoll ep;
    Channel *serverchannel = new Channel(&ep,servsock.fd(),true);
    serverchannel->enablereading();

    while (true)
    {
        std::vector<Channel *> channels = ep.loop();
        for (auto &ch:channels)
        {
            ch->handleevent(&servsock);
        }
    }
    return 0;
}