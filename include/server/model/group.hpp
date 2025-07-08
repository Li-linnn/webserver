#ifndef GROUP_H
#define GROUP_H
#include<string>
#include<vector>
#include"groupuser.hpp"

class Group
{
public:
    Group(int id = -1, std::string group_name = " ", std::string group_desc = " ")
    {
        this->group_id = id;
        this->group_name = group_name;
        this->group_desc = group_desc;
    }
    void setID(int id){this->group_id = id;}
    void setName(std::string name){this->group_name = name;}
    void setDesc(std::string desc){this->group_desc = desc;}

    int getID(){return group_id;}
    std::string getName(){return group_name;}
    std::string getDesc(){return group_desc;}
    std::vector<GroupUser>& getUsers(){return this->users;}
private:
    int group_id;
    std::string group_name;
    std::string group_desc;
    /*
    用于返回群内成员，而群内成员不仅需要User类的性质，还需要GroupUser表内的grouprole性质，
    所以新建一种GroupUser类
    */
    std::vector<GroupUser> users;
};

#endif