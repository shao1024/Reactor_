#include "TcpServer.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("usage: ./tcpepoll ip port\n"); 
        printf("example: ./bin/tcpserver_v1 172.25.104.182 5005\n\n"); 
        return -1;
    }

    TcpServer tcpserver(argv[1],atoi(argv[2]));
    tcpserver.start();

    
    return 0;
}