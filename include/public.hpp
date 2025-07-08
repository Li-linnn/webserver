#ifndef PUBLIC_H
#define PUBLIC_H
#include<iostream>

enum EnMsgType //定义枚举
{
    LOGIN_MSG = 1,//登录
    REG_MSG,//注册
    REG_MSG_ACK,//注册返回
    LOGIN_MSG_ACK,//登录返回
    LOGINOUT_MSG,//注销
    LOGINOUT_MSG_ACK,//注销回复
    ONE_CHAT_MSG,//一对一聊天,
    ONE_CHAT_MSG_ACK,//一对一聊天返回
    ADD_FRIEND_MSG,//添加好友

    CREATE_GROUP_MSG,//创建群组
    ADD_GROUP_MSG,//加入群组
    GROUP_CHAT_MSG,//群组聊天
    GROUP_CHAT_MSG_ACK//群组聊天返回
}; 

//定义宏
#define LOG(str) \
    std::cout << __FILE__ << ":" << __LINE__ << " " << \
    __TIMESTAMP__ << " : " << str << std::endl;
#endif