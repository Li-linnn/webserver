#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include<unordered_map>
#include<functional>
#include<muduo/net/TcpServer.h>
#include"json.hpp"
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
#include"usermodel.hpp"
#include<mutex>
#include"offlineMsgmodel.hpp"
#include"friendmodel.hpp"
#include"groupmodel.hpp"
#include"redis.hpp"
#include"Connection.hpp"
#include"CommonConnectionPool.hpp"

//业务处理器函数
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp)>;

//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr& conn, json &js, Timestamp);
    //处理注册业务
    void reg(const TcpConnectionPtr& conn, json &js, Timestamp);
    //注销业务
    void loginout(const TcpConnectionPtr& conn, json &js, Timestamp);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理用户异常退出
    void clientCloseEception(const TcpConnectionPtr& conn);
    //一对一聊天服务
    void oneChat(const TcpConnectionPtr& conn, json &js, Timestamp);
    //服务器异常，业务重置方法
    void reset();
    //添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json &js, Timestamp);
    //创建群组
    void createGroup(const TcpConnectionPtr& conn, json &js, Timestamp);
    //加入群组
    void addGroup(const TcpConnectionPtr& conn, json &js, Timestamp);
    //群组聊天
    void ChatGroup(const TcpConnectionPtr& conn, json &js, Timestamp);
    //用于初始化redis中的_notify_message_handler
    void handleRedisSubscribeMessage(int channel, std::string msg);
private:
    ChatService();
    //存储消息id和其对应的业务处理方法
    std::unordered_map<int, MsgHandler> _handlerMap;
    //存储在线用户, int:用户id
    std::unordered_map<int, TcpConnectionPtr> _userConnMap;//可以多个id对应一个conn吧？
    //定义互斥锁
    std::mutex _connMutex;
    //User表的操作接口
    UserModel _userModel;
    //OfflineMesssage表的操作接口
    offlineMsgModel _offlineMsgModel;
    //friend表的操作接口
    friendModel _friendModel;
    //Group表操作接口
    GroupModel _groupModel;
    //redis
    Redis _redis;
    //连接池
   // ConnectionPool* _pool;
};

#endif