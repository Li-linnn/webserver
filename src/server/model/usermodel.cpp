#include"usermodel.hpp"
#include"user.hpp"
//#include"db.h"
#include<mysql/mysql.h>
#include<iostream>
#include<muduo/base/Logging.h>
#include"CommonConnectionPool.hpp"
#include"Connection.hpp"

//直接封装insert方法，用户只需要输入信息，无需编写sql语句
bool UserModel::insert(User& user)
{
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into users(username, password, state) values('%s', '%s', '%s')",
        user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    auto pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    if(mysql_ptr->update(sql))
    {
        //获取插入成功的用户获取的主键id
        user.setId(mysql_insert_id(mysql_ptr->getMySQL()));
        return true;
    }
    return false;
    /*
    MySQL mysql;
    //MySQL封装了数据库的连接操作（且自动连接）和更新、查询操作
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户获取的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
    */
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from users where id = %d", id);

    auto pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    MYSQL_RES* res = mysql_ptr->query(sql);
    if(res != nullptr)
        {
            LOG_INFO << "该用户已注册";
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
                /*
                为什么需要 mysql_free_result(res)？​​
                    ​​结果集内存由 MySQL 库分配​​
                    当调用 mysql_query() 执行 SELECT 语句后，内部调用 mysql_store_result() 
                    或 mysql_use_result() 获取结果集。这两个函数会在 ​​MySQL 库内部动态分配内
                    存​​存储结果数据，并返回 MYSQL_RES* 指针。​​必须显式释放内存​​
                    如果未调用 mysql_free_result()，这部分内存会 ​​泄漏​​，因为 MySQL 库无法自动回收它分配的内存。
                */
            }
        }
        else
        {
            LOG_INFO << "该用户未注册";
        }
}
/*
MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            LOG_INFO << "该用户已注册";
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                User user;//创建一个user储存查询到的信息返回回去
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
                
            }
        }
        else
        {
            LOG_INFO << "该用户未注册";
        }
    }
*/
    
bool UserModel::update_state(User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "update users set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    
    auto pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    if(mysql_ptr->update(sql))
    {
        return true;
    }
    return false;
    /*
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
    */
}

void UserModel::resetState()
{
    char sql[1024] = {"update users set state = 'offline' where state = 'online'"};
    
    auto pool = ConnectionPool::getConnectionPool();
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