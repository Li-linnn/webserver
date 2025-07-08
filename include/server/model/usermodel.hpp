#ifndef USERMODEL_H
#define USERMODEL_H
#include"user.hpp"
#include"Connection.hpp"
#include"CommonConnectionPool.hpp"

//User表的操作类
class UserModel
{
public:
    //User表的插入功能
    bool insert(User& user);
    //查询功能
    User query(int id);
    //更新功能
    bool update_state(User& user);
    //重置用户的状态信息
    void resetState();
private:
};

#endif