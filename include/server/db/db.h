#ifndef DB_H
#define DB_H

#include<mysql/mysql.h>
#include<string>
using namespace std;

//数据库操作类：给出连接及操作数据库的接口
class MySQL
{
public:
    //初始化数据库连接
    MySQL();
    //释放数据库连接
    ~MySQL();
    //连接数据库
    bool connect();
    //更新操作
    bool update(string sql);
    //查询操作
    MYSQL_RES* query(string sql);
    //获取真正创建的数据库
    MYSQL* getConnection();
private:
    MYSQL* _conn;//指向通过mysql_init()在客户端堆区创建的MYSQL对象
};
#endif

