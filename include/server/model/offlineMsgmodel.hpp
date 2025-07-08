#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H
#include<string>
#include<vector>
#include<mutex>

class offlineMsgModel
{
public:
    //存储离线消息
    void insert(int userid, std::string msg);
    //删除用户离线消息
    void remove(int userid);
    //查询用户离线消息
    std::vector<std::string> query(int userid);
private:
};

#endif