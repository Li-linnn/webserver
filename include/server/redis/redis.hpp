#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

class Redis
{
public:
    Redis();
    ~Redis();

    //连接redis
    bool connect();
    //发布消息
    bool publish(int channel, std::string msg);
    //接收消息
    void observer_channel_message();
    //订阅通道
    bool subscribe(int channel);
    //解除订阅
    bool unsubscribe(int channel);
    //初始化通知消息的回调函数
    void init_notify_handler(std::function<void(int,std::string)> fn);
private:
    //负责pulish的上下文
    redisContext* _publish_context;
    //负责subscribe的上下文
    redisContext* _subscribe_context;
    //收到消息后的处理函数
    std::function<void(int,std::string)> _notify_message_handler;
};

#endif