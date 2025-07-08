#ifndef CONNECTION_H
#define CONNECTION_H
#include<mysql/mysql.h>
#include<string>
using namespace std;

class Connection
{
public:
    //初始化数据库连接
    Connection();
    //释放数据库连接
    ~Connection();
    //连接数据库
    bool connect(string ip, unsigned short port, string usrname, string password, 
                string dbname);
    //更新操作
    bool update(string sql);
    //查询操作
    MYSQL_RES* query(string sql);
    //获取数据库实例指针
    MYSQL* getMySQL();
    //刷新连接的起始时间
    void refreshAliveTime(){ _alivetime = clock(); }
    //返回空闲时间
    clock_t getAliveTime(){ return clock() - _alivetime; }
private:
    MYSQL* _conn;
    //连接起始时间
    clock_t _alivetime;
};

#endif