#include"db.h"
#include <muduo/base/Logging.h>

//数据库配置信息
static std::string server = "127.0.0.1";
static std::string user = "root";//mysql用户账号
static std::string password = "308418";//mysql密码
static std::string dbname = "chat";//数据库名称

//初始化数据库连接
MySQL::MySQL()
{
    //如果传入null，就分配内存，在客户端堆区创建一个新的MySQL对象，_conn指向该对象
    _conn = mysql_init(nullptr);
}
//释放数据库连接
MySQL::~MySQL()
{
    if(_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
//连接数据库
bool MySQL::connect()
{
    //mysql_real_connect直接修改传入的 _conn 对象，将客户端mysql对象和服务端mysql连接
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                         password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if(p != nullptr)
    {
        //向 MySQL 服务器发送并执行一条 SQL 查询
        mysql_query(_conn, "set names gbk");
        /*
        1、客户端发送的中文字符会按 gbk 编码传输到服务器
        2、服务器会将客户端发来的 gbk 编码的 SQL 语句，转换为该字符集后再处理
        3、服务器返回的中文字符会按 gbk 编码发送给客户端。
        */
        LOG_INFO << "connect mysql seccess!";
        return true;
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
        return false;
    }
}
//更新操作
bool MySQL::update(string sql)
{
    //mysql_query执行成功返回0，执行失败返回非0
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    }
    return true;
}
//查询操作
MYSQL_RES* MySQL::query(string sql)
{
    //mysql_query执行成功返回0，执行失败返回非0
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }
    //逐行从服务器读取查询结果​​
    return mysql_use_result(_conn);
}
//获取真正的mysql
MYSQL* MySQL::getConnection()
{
    return _conn;
}