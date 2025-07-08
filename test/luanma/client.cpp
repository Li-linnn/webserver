#include<iostream>
#include<string.h>
#include<cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>

int main()
{  
    //创建socket
    int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sockfd<0)
    {
        printf("create socket error: error=%d errmsg=%s\n", errno, strerror(errno));
        return 1;
    }
    else
    {
        printf("create socket success!\n");
    }
    //连接服务端
    std::string ip = "127.0.0.1";
    int port = 8080;

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    sockaddr.sin_port = htons(port);

    if(::connect(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    {
        printf("error connect");
    }
    else
    {
        printf("success connect");
    }
    //一旦connect成功，客户端的sockfd就自动和服务端的connfd绑定

    //向服务端发送数据
    std::string data = "hello world!";
    ::send(sockfd, data.c_str(), data.size(), 0);

    //接收服务端数据
    char buf[1024] = {0};
    ::recv(sockfd, buf, sizeof(buf), 0);
    printf("recv success");

    //关闭
    ::close(sockfd);

    return 0;
}