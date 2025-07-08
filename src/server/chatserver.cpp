#include"chatserver.hpp"
#include<functional>
using namespace std::placeholders;
#include<string>
#include"json.hpp"
using json = nlohmann::json;
#include"chatservice.hpp"

ChatServer::ChatServer(EventLoop *loop,//事件循环
    const InetAddress &listenAddr,
    const string &nameArg)
    :_server(loop, listenAddr, nameArg), _loop(loop)
    {

        // TcpServer _server 内部会创建监听 Socket，并绑定到指定地址和端口
        //注册链接回调函数,注意&ChatServer::onConnection，不能直接写onConnection，出过错
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

        //注册消息回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        //设置线程数量，在主eventloop线程内部创建n个子EventLoop线程
        _server.setThreadNum(4);
    }

//启动服务
void ChatServer::start()
{
    _server.start();
}

//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
/*
    conn: 连接 Socket 被包装为 TcpConnection 对象（即 conn）

    ​​连接 Socket 的创建​​：
        当客户端完成三次握手后，主EventLoop 检测到监听 Socket 的可读事件（新连接到达），
    调用 accept() 创建连接 Socket。

    ​​触发回调​​：
        连接 Socket 被包装为 TcpConnection 对象（即 conn），并触发 onConnection 回调(conn)，
    连接好的socket送到子reactor
*/

{
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseEception(conn);
        conn->shutdown();
    }
}
//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                Buffer* buffer,
                Timestamp time)
{
    // 当连接 Socket 有数据到达时，muduo 自动读取数据到 Buffer，并触发 onMessage 回调：
    string buf = buffer->retrieveAllAsString();
    //数据反序列化
    json js = json::parse(buf);

    //通过js["msgid"]获取对应的handler
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //调用handler 
    msgHandler(conn, js, time);
}
