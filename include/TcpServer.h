#pragma once

#include "Acceptor.h"
#include "ThreadPool.h"
#include <map>
#include <memory>

// Tcp网络服务类
class TcpServer
{
private:
    // 线程池的大小，即从事件循环的个数  
    // (注：在使用初始化列表将参数作为其它类初始化的参数，需保证在头文件中该参数在那个类的前面)
    // 如：使用threadnum_初始化threadpool_时，在类的声明中中threadnum_需要在threadpool_的前面
    int threadnum_;
    // 主事件循环。
    std::unique_ptr<EventLoop> mainloop_;
    // 存放从事件循环的容器
    std::vector<std::unique_ptr<EventLoop>> subloops_;
    // 一个TcpServer只有一个Acceptor对象。
    Acceptor acceptor_;
    // 线程池
    ThreadPool threadpool_;
    
    
    // 一个TcpServer有多个Connection对象，存放在map容器中;int表示套接字以及其对应的connection
    std::map<int,spConnection> conns_;

    // 回调EchoServer::HandleNewConnection()
    std::function<void(spConnection)> newconnectioncb_;
    // 回调EchoServer::HandleClose()
    std::function<void(spConnection)> closeconnectioncb_;
    // 回调EchoServer::HandleError()
    std::function<void(spConnection)> errorconnectioncb_;
    // 回调EchoServer::HandleMessage()   注意第二个参数采用的是传引用的方式，当message中的数据量大时，这种方式速度更快
    std::function<void(spConnection,std::string &message)> onmessagecb_;
    // 回调EchoServer::HandleSendComplete()
    std::function<void(spConnection)> sendcompletecb_;
    // 回调EchoServer::HandleTimeOut()
    std::function<void(EventLoop*)> epolltimeoutcb_;
    
    
    
public:
    TcpServer(const std::string &ip, const uint16_t port, int threadnum=3);
    ~TcpServer();

    // 运行事件循环
    void start();

    // 处理新客户端连接请求,创建connection便于tcpserver管理
    void newconnection(std::unique_ptr<Socket> clientsock);
    // 关闭客户端的连接，在Connection类中回调此函数
    void closeconnection(spConnection conn);
    // 客户端的连接错误，在Connection类中回调此函数
    void errorconnection(spConnection conn);
    // 处理客户端的请求报文，在Connection类中回调此函数
    void onmessage(spConnection conn,std::string& message);
    // 数据发送完成后，在Connection类中回调此函数
    void sendcomplete(spConnection conn);
    // epoll_wait()超时，在EventLoop类中回调此函数
    void epolltimeout(EventLoop* loop);

    void setnewconnectioncb(std::function<void(spConnection)> fn);
    void setcloseconnectioncb(std::function<void(spConnection)> fn);
    void seterrorconnectioncb(std::function<void(spConnection)> fn);
    void setonmessagecb(std::function<void(spConnection,std::string &message)> fn);
    void setsendcompletecb(std::function<void(spConnection)> fn);
    void setepolltimeoutcb(std::function<void(EventLoop*)> fn);



};




