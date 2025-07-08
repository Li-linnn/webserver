#include"friendmodel.hpp"
//#include"db.h"
#include<vector>
#include"CommonConnectionPool.hpp"
#include"Connection.hpp"

void friendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);

    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    mysql_ptr->update(sql);
/*
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
*/
}

std::vector<User> friendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.username, a.state, b.userid from users a inner join" 
                 " friend b on a.id = b.friendid where b.userid = %d", userid);

    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    std::vector<User> vec;
    MYSQL_RES* res = mysql_ptr->query(sql);//MYSQL_RES 是一个结构体，表示 ​​完整的查询结果集
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while(row = mysql_fetch_row(res))
            {
                if(row != nullptr)
                {
                    User user;//创建一个user储存查询到的信息
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    vec.push_back(user);
                }
            }
            mysql_free_result(res);
            return vec;
        }
/*
MySQL mysql;
    std::vector<User> vec;
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
                    User user;//创建一个user储存查询到的信息返回回去
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    vec.push_back(user);
                }
            }
            mysql_free_result(res);
            return vec;
        }
    }
*/
}