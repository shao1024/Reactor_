#include "InetAddress.h"


InetAddress::InetAddress()
{

}

// 如果是监听的fd，用这个构造函数
InetAddress::InetAddress(const std::string &ip,uint16_t port)
{
    // IPv4网络协议的套接字类型
    addr_.sin_family = AF_INET;
    // 服务端用于监听的ip地址   inet_addr函数将点分十进制的IP地址字符串转换为32位的IPv4地址整数表示
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    // htons主机字节序转换为网络字节序
    addr_.sin_port = htons(port);

}  

// 如果是客户端连上来的fd，用这个构造函数
InetAddress::InetAddress(const sockaddr_in addr)
{
    addr_ = addr;
}         

InetAddress::~InetAddress()
{

}

// 返回字符串表示的地址，例如：192.168.150.128
const char* InetAddress::ip() const
{
    // inet_ntoa函数与inet_addr相反，将32位的整数IPv4地址转换为字符串
    return inet_ntoa(addr_.sin_addr); 
}

// 返回整数表示的端口，例如：80、8080
uint16_t InetAddress::port() const
{
    // ntohs与htons相反，将网络字节序转换为主机字节序
    return ntohs(addr_.sin_port); 
}

// 返回addr_成员的地址，转换成了sockaddr。
const sockaddr* InetAddress::addr() const
{
    return (sockaddr*)&addr_; 
}

// 设置addr_成员的值。
void InetAddress::setaddr(sockaddr_in clientaddr)
{
    addr_ = clientaddr;
}