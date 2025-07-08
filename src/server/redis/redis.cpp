#include"redis.hpp"
#include"iostream"

Redis::Redis():_publish_context(nullptr), _subscribe_context(nullptr){}
Redis::~Redis()
{
    if(_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

//连接redis
bool Redis::connect()
{
    //负责publish的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if(_publish_context == nullptr)
    {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }
    //负责subscribe的上下文连接
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if(_publish_context == nullptr)
    {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }

    std::thread t([&](){
        observer_channel_message();
    });
    t.detach();
    std::cout << "connect redis-server success!" << std::endl;
    return true;
}

//发布消息
//相当于封装一下发送消息功能，否则需要用户自己调用redisCommand
bool Redis::publish(int channel, std::string msg)
{
    redisReply* reply = (redisReply*)redisCommand(_publish_context, "PUBLISH %d %s", channel, msg.c_str());
    //redisReply*：指向redis响应的结构体
    if(reply == nullptr)
    {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

//接收消息
void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while(REDIS_OK == redisGetReply(this->_subscribe_context, (void**)&reply))
    //当收到消息时，返回 REDIS_OK 并将消息解析到 reply 对象中
    {
        //在订阅模式下，redisGetReply 会 ​​阻塞等待​​，直到 Redis 服务器推送消息到订阅的频道。
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            /*
            订阅消息的 reply 是一个数组，结构为 ["message", "频道名", "消息内容"]。
            element[0]：固定为字符串 "message"。
            element[1]：频道名（代码中转为整数，假设频道名为数字）。
            element[2]：实际消息内容。
            */

           //调用给channel发送消息的函数
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    std::cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>>>observer_channel_message quit <<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
}
//订阅通道
bool Redis::subscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel))
    //redisAppendCommand先将命令存到缓冲区
    {
        std::cerr << "subscribe command failed!" <<std::endl;
        return false;
    }
    int done = 0;
    //缓冲区数据全部发送完 done 被置为1
    while (!done)
    {
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        //将缓冲区内容发送给redis-server
        {
            std::cerr << "subscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}
/*
这里为什么不和publish一样利用redisCommand发送命令：
    redisReply *redisCommand(redisContext *c, const char *format, ...) 
    {
    // 1. 格式化命令并写入缓冲区
    if (redisAppendCommand(c, format, ...) != REDIS_OK) return NULL;

    // 2. 立即发送命令并等待响应
    redisReply *reply;
    if (redisGetReply(c, (void**)&reply) != REDIS_OK) return NULL;
    return reply;
    }
    
    redisCommand 发送 SUBSCRIBE 后，会调用 redisGetReply ​​等待服务器响应​​。
    然而，在订阅模式下，服务器不会立即返回响应，而是持续推送消息（将该通道收到
    的消息推送给订阅者），导致 redisGetReply ​​无限阻塞​​，后续代码无法执行。
*/

//解除订阅
bool Redis::unsubscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel))
    //redisAppendCommand先将命令存到缓冲区
    {
        std::cerr << "unsubscribe command failed!" <<std::endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        //将缓冲区内容发送给redis-server
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}
//初始化回调函数，相当于让用户自定义_notify_message_handler
void Redis::init_notify_handler(std::function<void(int, std::string)> fn)
{
    this->_notify_message_handler = fn;
}