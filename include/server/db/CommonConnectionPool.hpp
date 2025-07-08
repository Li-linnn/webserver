#ifndef COMMONCONNECTIONPOOL_H
#define COMMONCONNECTIONPOOL_H
#include<string>
#include<queue>
#include<Connection.hpp>
#include<mutex>
#include<atomic>
#include<iostream>
#include<memory>
#include<condition_variable>
#include<thread>

class ConnectionPool
{
public:
    //获取单例对象
    static ConnectionPool* getConnectionPool();
    
    //获取连接池中的连接
    //static Connection* getConnection();
    shared_ptr<Connection> getConnection();
    //连接需要用到成员变量，若设计为静态函数，不能访问成员变量

    //析构函数
    ~ConnectionPool();
private:
    ConnectionPool();
    //从配置文件中加载配置项
    bool loadConfigFile();
    //生产连接函数
    void produceConnectionTask();
    //扫描超时连接函数
    void scannerConnectionTask();

    std::string _ip;
    unsigned short _port;
    std::string _usrname;
    std::string _password;
    std::string _dbname;
    int _initSize;//初始连接量
    int _maxSize;//最大连接量
    int _maxIdleTime;//最大空闲时间
    int _connectionTimeout;//超时连接时间

    std::queue<Connection*> _connectionQue;
    mutex _queueMutex;
    atomic_int _connectionCnt;
    //创建的连接总数量（初始+后续创建）
    //原子性整型，相当于自动加锁保证了线程安全性
    std::condition_variable cv;

    std::thread _produceThread;
    std::thread _scannerThread;
    std::atomic<bool> _running{true};
    //用于通知线程结束
};

#endif