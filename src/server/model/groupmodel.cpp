#include"groupmodel.hpp"
//#include"db.h"
#include"group.hpp"
#include<muduo/base/Logging.h>
#include"CommonConnectionPool.hpp"
#include"Connection.hpp"

//创建群
bool GroupModel::createGroup(Group& group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')", 
        group.getName().c_str(), group.getDesc().c_str());

    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    if(mysql_ptr->update(sql))
    {
        group.setID(mysql_insert_id(mysql_ptr->getMySQL()));
        LOG_INFO << "创建群组成功";
        return true; 
    }
    return false;
    /*
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setID(mysql_insert_id(mysql.getConnection()));
            LOG_INFO << "创建群组成功";
            return true;
        }
    }
    return false;
    */
}

//加入群
bool GroupModel::addGroup(int userid, int groupid, std::string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')", groupid, userid, role.c_str());

    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    if(mysql_ptr->update(sql))
    {
        LOG_INFO << "加入群组成功";
        return true;
    }
    return false;
/*
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            LOG_INFO << "加入群组成功";
            return true;
        }
    }
    return false;
*/
}

//查询用户所在群组信息
std::vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from AllGroup a inner join "
        "GroupUser b on a.id = b.groupid where b.userid = %d", userid);

    std::vector<Group> vec;
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    MYSQL_RES* res = mysql_ptr->query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while(row = mysql_fetch_row(res))
            {
                if(row != nullptr)
                {
                    Group group;
                    group.setID(atoi(row[0]));
                    group.setName(row[1]);
                    group.setDesc(row[2]);
                    vec.push_back(group);
                    /*
                    int groupid = group.getID();
                    std::vector<GroupUser> usersVec = _groupModel.queryGroupUsers(id, groupid);
                    for(GroupUser& user : usersVec)
                    {
                        group.addUser(user);
                    }
                    */
                }
            }
            mysql_free_result(res);
            return vec;
        }
/*
 MySQL mysql;    
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);//MYSQL_RES 是一个结构体，表示 ​​完整的查询结果集
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while(row = mysql_fetch_row(res))
            {
                if(row != nullptr)
                {
                    //先创建一个群组类，组装信息返回回去
                    Group group;
                    group.setID(atoi(row[0]));
                    group.setName(row[1]);
                    group.setDesc(row[2]);
                    vec.push_back(group);
                }
            }
            mysql_free_result(res);
            return vec;
        }
    }
*/
}

//根据groupid查询群内除本人外其他用户id
std::vector<GroupUser> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.username, a.state, b.grouprole from users a inner \
        join GroupUser b on a.id = b.userid where b.groupid = %d and b.userid != %d", 
        groupid, userid);

    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    std::vector<GroupUser> vec;
    MYSQL_RES* res = mysql_ptr->query(sql);
    if(res != nullptr)
    {
        MYSQL_ROW row;
        while(row = mysql_fetch_row(res))
        {
            if(row != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                vec.push_back(user);
            }
        }
        mysql_free_result(res);
        return vec;
    }
/*
    MySQL mysql;
    std::vector<GroupUser> vec;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while(row = mysql_fetch_row(res))
            {
                if(row != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    vec.push_back(user);
                }
            }
            mysql_free_result(res);
            return vec;
        }
    }
*/
}