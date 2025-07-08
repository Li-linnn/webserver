#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include<string>
#include<vector>
#include"group.hpp"

class GroupModel
{
public:
    //创建群
    bool createGroup(Group& group);
    //加入群
    bool addGroup(int userid, int groupid, std::string role);
    //查询用户所在群组信息
    std::vector<Group> queryGroups(int userid);
    //根据groupid查询用户列表
    std::vector<GroupUser> queryGroupUsers(int userid, int groupid);
private:
};

#endif