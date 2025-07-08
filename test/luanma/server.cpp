#include<iostream>
#include<sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include<unistd.h>
int main()
{
    //1、创建socket
    int sockfd=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);//"::"利用全局下的socket
    //参数列表：协议簇，套接字类型，协议类型
    //创建失败返回-1，创建成功返回fd
    //sockfd是个文件描述符数组下标，该位置存储一个指针，该指针指向创建的socket对象
    if(sockfd<0)
    {
        printf("create socket error: error=%d errmsg=%s\n", errno, strerror(errno));
        return 1;
    }
    else
    {
        printf("create socket success!\n");
    }
    
    //绑定socket
    std::string ip="127.0.0.1";
    int port = 8080;

    struct sockaddr_in sockaddr;//sockaddr_in:用来封装IP地址、端口号和地址类型的结构体
    memset(&sockaddr, 0, sizeof(sockaddr));//memset:用来初始化
    sockaddr.sin_family = AF_INET;//地址类型
    sockaddr.sin_addr.s_addr = inet_addr(ip.c_str());//inet_addr将网络主机地址转换为网络字节序二进制值
    sockaddr.sin_port = htons(port);//端口号，htons:将一个16位数从主机字节顺序转换成网络字节顺序
    if(::bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    //将封装好的ip和端口号绑定到socket，成功返回0，失败返回-1
    {
        printf("socket bind error: error=%d errmsg=%s\n", errno, strerror(errno));
        return 1;
    }
    else
    {
        printf("socket bind success: ip=%s port=%d\n", ip.c_str(), port);
    }

    //监听socket
    if(::listen(sockfd, 1024) < 0)
    {
        printf("error listen\n");
        return 1;
    }
    else
    {
        printf("listen\n");
    }

    while(true)//通过 accept()在循环中持续获取新连接，使服务器不会在处理完单个客户端后退出。
    {
        //接收客户端连接
        int connfd = ::accept(sockfd, nullptr, nullptr);
        //accept：创建一个新套接字，继承sockfd的属性，返回该新套接字的index即connfd
        if(connfd < 0)
        {
            printf("error connect\n");
            return 1;
        }
        
        char buf[1024] = {0};
        //接收客户端数据
        size_t len = ::recv(connfd, buf, sizeof(buf), 0);
        /*
        recv返回值：
            正数：表示接收到n字节的数据
            0：表示正常关闭
            -1：错误
        */
        printf("success\n");
        //向客户端发送数据
        ::send(connfd, buf, len, 0);
    }

    //关闭socket
    ::close(sockfd);
    return 0;
}