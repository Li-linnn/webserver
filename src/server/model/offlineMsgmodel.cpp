#include"offlineMsgmodel.hpp"
#include<vector>
#include<string>
//#include"db.h"
#include<mutex>
#include"CommonConnectionPool.hpp"
#include"Connection.hpp"

void offlineMsgModel::insert(int userid, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

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

void offlineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

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

std::vector<std::string> offlineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<std::string> vec;
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    std::shared_ptr<Connection> mysql_ptr = pool->getConnection();
    MYSQL_RES* res = mysql_ptr->query(sql);
    if(res != nullptr)
    {
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr)/*while(row = mysql_fetch_row(res) != nullptr) 执行的是：row = (mysql_fetch_row(res) != nullptr)*/
        {
            vec.push_back(row[0]);
        }
        
        mysql_free_result(res);
        return vec;
        /*
        用vector接收而非string直接接收的原因：
            聊天服务器中，一个用户通常会有 ​​多条离线消息，比如多次insert
            SELECT 查询会返回多行数据，必须用 vector存储所有结果
        */
    }

/*
MySQL mysql;
if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            //while(row = mysql_fetch_row(res) != nullptr) 执行的是：row = (mysql_fetch_row(res) != nullptr)
            {
                vec.push_back(row[0]);
            }       
            
            mysql_free_result(res);
            return vec;    
        }
    }
*/ 
}