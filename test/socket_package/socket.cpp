#include<socket_package/socket.h>
using namespace yazi::socket;

Socket::Socket() : m_ip(""), m_port(0), m_sockfd(0)
{
    //1、创建socket
    m_sockfd=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);//"::"利用全局下的socket
    //参数列表：协议簇，套接字类型，协议类型
    //创建失败返回-1，创建成功返回fd
    //sockfd是个文件描述符数组下标，该位置存储一个指针，该指针指向创建的socket对象
    if(m_sockfd<0)
    {
        //printf("create socket error: error=%d errmsg=%s\n", errno, strerror(errno));
        //实际封装的时候，将错误写进日志
        log_error("create socket error: error=%d errmsg=%s", errno, strerror(errno));
        //return 1;
    }
    else
    {
        log_debug("create socket success!");
    }
}

Socket::Socket(int sockfd) : m_ip(""), m_port(0), m_sockfd(0)
{

}

bool Socket::bind(const string& ip, int port)
{
    m_ip = ip;
    m_port = port;
    struct sockaddr_in sockaddr;//sockaddr_in:用来封装IP地址、端口号和地址类型的结构体
    memset(&sockaddr, 0, sizeof(sockaddr));//memset:用来初始化
    sockaddr.sin_family = AF_INET;//地址类型
    if(m_ip.empty())
    {
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);//若ip为空，绑定主机任意ip
    }
    else
    {
        sockaddr.sin_addr.s_addr = inet_addr(m_ip.c_str());//inet_addr将网络主机地址转换为网络字节序二进制值
    }
    sockaddr.sin_port = htons(m_port);//端口号，htons:将一个16位数从主机字节顺序转换成网络字节顺序
    if(::bind(m_sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    //将封装好的ip和端口号绑定到socket，成功返回0，失败返回-1
    {
        log_error("socket bind error: error=%d errmsg=%s", errno, strerror(errno));
        return false;
    }
    log_debug("socket bind success: ip=%s port=%d", m_ip.c_str(), m_port);
}

bool Socket::listen(int backlog)
{
    //监听socket
    if(::listen(m_sockfd, 1024) < 0)
    {
        log_error("error listen");//下面都一样
        return flase;
    }
    log_debug("listen");
}

bool Socket::connect(const string& ip, int port)
{
    m_ip = ip;
    m_port = port;
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    sockaddr.sin_port = htons(m_port);

    if(::connect(m_sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    {
        printf("error connect");
    }
    else
    {
        printf("success connect");
    }
}

int Socket::accept()
{
    //接收客户端连接
    int connfd = ::accept(m_sockfd, nullptr, nullptr);
    //accept：创建一个新套接字，继承sockfd的属性，返回该新套接字的index即connfd
    if(connfd < 0)
    {
        printf("error connect\n");
        return 1;
    }
    log_debug("accept success!", connfd);
    return connfd;
}

int Socket::send(const char* buf, int len)
{
    return ::send(m_sockfd, buf, len, 0);
}

int Socket::recv(char* buf, int len)
{
    return ::recv(m_sockfd, buf, len, 0);
}

void Socket::close()
{
    if(m_sockfd > 0)
    {
        ::close(m_sockfd);
        m_sockfd = 0;
    }
}

Socket::~Socket()
{
    close();
}

