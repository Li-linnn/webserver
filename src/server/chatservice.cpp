#include"chatservice.hpp"
#include"public.hpp"
#include<string>
#include<muduo/base/Logging.h>
using namespace muduo;
using namespace std::placeholders;
#include"user.hpp"
#include"usermodel.hpp"
#include<mutex>
#include<vector>
#include<algorithm>
#include"groupuser.hpp"
#include"CommonConnectionPool.hpp"

//获取单例对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

/*
定义一个业务处理器容器，msgid-handler，通过消息传入的msgid调用对应的handler
*/
ChatService::ChatService()
{
    _handlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _handlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _handlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _handlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _handlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _handlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _handlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::ChatGroup, this, _1, _2, _3)});
    _handlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    //连接redis服务器
    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    } 
    /*
    handleRedisSubscribeMessage 由 ​​Redis 服务器的推送事件​​触发:
        init_notify_handler	注册回调函数，定义收到订阅消息时的处理逻辑。
        observer_channel_message	后台线程函数，阻塞等待 Redis 消息，触发回调。
    */   

    //创建连接池？
    //_pool = ConnectionPool::getConnectionPool();
}

//获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    //匹配msgid
    auto it = _handlerMap.find(msgid);

    if(it==_handlerMap.end())
    {
        //找不着handler，返回一个默认的空处理器
        return [=](const TcpConnectionPtr& conn, json &js, Timestamp)->void{
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    }
    else
    {
        return _handlerMap[msgid];
    }
}
//处理登录业务的handler
void ChatService::login(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id && user.getPassword() == password)
    {
        /*
        _userConnMap是一个全局变量，所有线程共用这一个map，加锁保证线程安全
        */
        if(user.getState() == "online")
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "已登录，无需重复登录！";
            conn->send(response.dump());
        }
        else
        {
            user.setState("online");
            _userModel.update_state(user);
            {
                //容器操作要保证线程安全，mysql无需加锁
                //使用lock_guard:当线程离开作用域后，互斥锁自动释放
                std::lock_guard<std::mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            //登陆成功，向redis订阅channel（id号）
            _redis.subscribe(id);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询离线消息
            std::vector<std::string> offMsgVec = _offlineMsgModel.query(user.getId());
            if(!offMsgVec.empty())
            {
                response["offline message"] = offMsgVec;
                //读取后删除离线消息，保证离线消息只读取一次
                _offlineMsgModel.remove(user.getId());
            }
            
            //查询好友列表并返回
            std::vector<User> friendVec = _friendModel.query(user.getId());
            if(!friendVec.empty())
            {
                std::vector<std::string> vec;
                for(User& user : friendVec)
                {
                    json js;
                    js["friendid"] = user.getId();
                    js["friendname"] = user.getName();
                    js["friendstate"] = user.getState();
                    vec.push_back(js.dump());
                }
                response["friend"] = vec;
            }

            //查询群组信息并返回
            std::vector<Group> groupVec = _groupModel.queryGroups(id);
            if(!groupVec.empty())
            {
                std::vector<string> groupMsgVec;
                for(Group& group : groupVec)
                {
                    json groupjs;
                    groupjs["groupid"] = group.getID();
                    groupjs["groupname"] = group.getName();
                    groupjs["groupdesc"] = group.getDesc();

                    int groupid = group.getID();
                    std::vector<GroupUser> userVec = _groupModel.queryGroupUsers(id, groupid);
                    std::vector<string> userMsgVec;
                    for(GroupUser& groupuser : userVec)
                    {
                        json userjs;
                        userjs["userid"] = groupuser.getId();
                        userjs["username"] = groupuser.getName();
                        userjs["userstate"] = groupuser.getState();
                        userjs["userrole"] = groupuser.getRole();
                        userMsgVec.push_back(userjs.dump());
                    }
                    groupjs["groupuser"] = userMsgVec;
                    groupMsgVec.push_back(groupjs.dump());
                }
                response["group"] = groupMsgVec;
            }
            conn->send(response.dump());
        }
    }
}
//处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);
    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

//注销业务
void ChatService::loginout(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    /*
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if(it->second == conn)
            {
                user.setId(it->first);
                //将该用户从在线列表移除
                _userConnMap.erase(it);
                LOG_INFO << "用户已下线";
                break;
            }
        }
    }
    //更改数据库表中状态
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.update_state(user);
        LOG_INFO << "用户注销";
    }
    */
    int id = js["id"];
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if(it->first == id)
            {
                user.setId(it->first);
                //将该用户从在线列表移除
                _userConnMap.erase(it);
                LOG_INFO << "用户已下线";
                break;
            }
        }
    }
    //更改数据库表中状态
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.update_state(user);
        LOG_INFO << "用户注销";

        User reuser = _userModel.query(user.getId());
        json response;
        response["msgid"] = LOGINOUT_MSG_ACK;
        response["errno"] = 0; 
        response["id"] = reuser.getId();
        response["name"] = reuser.getName();
        conn->send(response.dump());
    }
    //注销后，unsubcribe
    _redis.unsubscribe(id);
}

//处理用户异常退出
void ChatService::clientCloseEception(const TcpConnectionPtr& conn)
{
    /*
    for(auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
    {
        if(it->second = conn)
        {
            User user;
            user.setId(it->first);
            user.setState("offline");
            _userModel.update_state(user);
            _userConnMap.erase(it);
            LOG_INFO << "用户异常退出";
            break;
        }
    }
    尽量还是将容器操作和mysql操作分开，因为容器操作需要加锁保证线程安全
    */
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if(it->second == conn)
            {
                user.setId(it->first);
                //将该用户从在线列表移除
                _userConnMap.erase(it);
                LOG_INFO << "用户已下线";
                break;
            }
        }
    }
    //更改数据库表中状态
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.update_state(user);
        LOG_INFO << "用户quit退出";
    }

    _redis.unsubscribe(user.getId());
}

//一对一聊天服务
void ChatService::oneChat(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    int id = js["id"].get<int>();
    std::string name = js["name"];
    int toid = js["toid"].get<int>();
    std::string msg = js["msg"];
    std::string time = js["time"];

    json response;
    response["msgid"] = ONE_CHAT_MSG_ACK;
    response["id"] = id;
    response["name"] = name;
    response["msg"] = msg;
    response["time"] = time;
    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if(it != _userConnMap.end())
    {
        LOG_INFO << "接收方用户已登录";
        //接收方用户在线
        it->second->send(response.dump());
    }
    else
    {
        User user = _userModel.query(toid);
        if(user.getState() == "online")
        {
            _redis.publish(toid, response.dump());
        }
        else
        {
            _offlineMsgModel.insert(toid, response.dump());
        }
    }
    // 这里将数据发送也框在锁内，首先确保不发生：数据发送过程中用户已下线情况
}

//服务器异常，业务重置方法
void ChatService::reset()
{
    //把online状态用户设置成offline
    _userModel.resetState();
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid);
}

//创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];

    Group group;
    group.setName(groupname);
    group.setDesc(groupdesc);
    if(_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getID(), "creator");
    }
}

//加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

//群组聊天
void ChatService::ChatGroup(const TcpConnectionPtr& conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    std::string name = js["name"];
    int groupid = js["groupid"].get<int>();
    std::string msg = js["msg"];
    std::string time = js["time"];

    json response;
    response["msgid"] = GROUP_CHAT_MSG_ACK;
    response["id"] = userid;
    response["name"] = name;
    response["groupid"] = groupid;
    response["msg"] = msg;
    response["time"] = time;

    std::vector<GroupUser> userVec = _groupModel.queryGroupUsers(userid, groupid);
    for(GroupUser& user : userVec)
    {
        int id = user.getId();
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            it->second->send(response.dump());
            LOG_INFO << "消息已发送";
        }
        else
        {
            User user = _userModel.query(id);
            if(user.getState() == "online")
            {
                _redis.publish(id, response.dump());
            }
            else
            {
                _offlineMsgModel.insert(id, response.dump());
            }
        }
    }
}

//从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int channel, std::string msg)
{
    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(channel);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    _offlineMsgModel.insert(channel, msg);
}