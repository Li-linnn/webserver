#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<iostream>
#include<functional>
using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;

//创建一个服务器类
class ChatServer
{
public:
    ChatServer(EventLoop *loop,//事件循环
            const InetAddress &listnAddr,
            const string &nameArg)
            :_server(loop, listnAddr, nameArg), _loop(loop)
    {
        //给服务器创建用户连接创建和断开的回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        /*
        这里绑定的原因：
        // 原始回调要求
        using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

        // 成员函数实际需要：
        void ChatServer::onConnection([隐式this], const TcpConnectionPtr&);
        */
        
        //给服务器注册用户读写事件的回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        //设置服务器端线程数量 muduo库会自己自动分配，此时一个线程负责用户连接，三个线程负责worker
        _server.setThreadNum(4);
    }

    //开启事件循环
    void start()
    {
        _server.start();
    }
private:
    //调用setConnectionCallback函数时，传参需要传入一个void（TcpConnectionPtr&）函数，所以：
    //专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr& conn){
        if(conn->connected())
        {
            std::cout << conn->peerAddress().toIpPort() << "->" 
            << conn->localAddress().toIpPort() << "state:online" << std::endl;
            //toIpPort:将封装的网络地址（包含IP和端口）转换为字符串
        }
        else
        {
            std::cout << conn->peerAddress().toIpPort() << "->" 
            << conn->localAddress().toIpPort() << "state:offline" << std::endl;
            conn->shutdown();
        }
    }
    //专门处理用户读写事件
    void onMessage(const TcpConnectionPtr& conn,
                Buffer* buffer,//缓冲区
                Timestamp time){
        string buf = buffer->retrieveAllAsString();
        std::cout << "recv data:" << buf << "time:" << time.toString() << std::endl;
        conn->send(buf);//测试：读什么返回什么
    }
    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};
int main()
{
    EventLoop loop;//类似创建一个epoll
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();//listenfd epoll_ctl
    loop.loop();//类似epoll_wait()
    return 0;
}