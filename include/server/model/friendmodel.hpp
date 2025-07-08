#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include"user.hpp"
#include<vector>

class friendModel
{
public:
    //添加好友
    void insert(int userid, int friendid);
    //返回好友列表
    std::vector<User> query(int userid);
};

#endif