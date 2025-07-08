#include<CommonConnectionPool.hpp>
#include"public.hpp"
#include<functional>
#include<muduo/base/Logging.h>

ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool;
    return &pool;
}

ConnectionPool::~ConnectionPool()
{
    LOG_INFO << "Pool执行析构";
    while(!_connectionQue.empty())
    {
        delete _connectionQue.front();
        //仅销毁堆上的Connection对象，不销毁该指针
        _connectionQue.pop();
    }

    // 设置运行标志为 false，通知线程退出循环
    _running = false;

    // 唤醒所有等待在条件变量上的线程
    cv.notify_all();
    LOG_INFO << "通知线程结束";

    // 等待生产者线程结束
    if (_produceThread.joinable()) {
        _produceThread.join();
    }

    // 等待扫描线程结束
    if (_scannerThread.joinable()) {
        _scannerThread.join();
    }
}

//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE* pf = fopen("mysql.conf", "r");
    /*
    fopen 是 C 标准库函数，用于打开文件, 打开模式为 ​​只读（r）
    成功打开文件 → 返回指向该文件的指针 pf 非空
    */
    if(pf == nullptr)
    {
        LOG("mysql.conf file is not exist!");
        return false;
    }
    while(!feof(pf))
    {
        char line[1024] = {0};
        fgets(line, 1024, pf);

        std::string str = line;
        int idx = str.find('=', 0);
        if(idx == -1)
        {
            continue;
        }

        int endidx = str.find('\n', idx);
        std::string key = str.substr(0, idx);
        std::string value = str.substr(idx + 1, endidx - (idx + 1));

        // 将配置项存入成员变量
        if (key == "ip") 
        {
            _ip = value;
        } 
        else if (key == "port") 
        {
            _port = atoi(value.c_str());
            //_port = std::stoi(value);
        } 
        else if (key == "usrname") 
        {
            _usrname = value;
        } 
        else if (key == "password") 
        {
            _password = value;
        }
        else if (key == "dbname") 
        {
            _dbname = value;
        }
        else if (key == "initSize") 
        {
            _initSize = atoi(value.c_str());
        } 
        else if (key == "maxSize") 
        {
            _maxSize = atoi(value.c_str());
        } 
        else if (key == "maxIdleTime") 
        {
            _maxIdleTime = atoi(value.c_str());
        } 
        else if (key == "connectionTimeOut") 
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

//连接池初始构造
ConnectionPool::ConnectionPool()
{
    //检查是否加载配置项
    if(!loadConfigFile())
    {
        return;
    }
    for(int i = 1; i < _initSize; i++)
    {
        Connection* p = new Connection();
        p->connect(_ip, _port, _usrname, _password, _dbname);
        _connectionQue.push(p);
        //连接入队了，刷新一下连接起始时间
        p->refreshAliveTime();
        _connectionCnt++;
    }

    //创建独立线程，生产连接
    _produceThread = ::thread (std::bind(&ConnectionPool::produceConnectionTask, this));
    /*
    std::thread 传入的参数需要是普通c风格函数，而非cpp成员函数，但如果设计一个全局函数，
    则函数无法访问ConnectionPool里的变量，所以设计成成员函数然后绑定this
    */
    produceThread.detach();
    /*
    很重要很重要！！！出现terminate called without an active exception的原因:
        当std::thread对象被销毁时，如果线程仍在运行且未被join()或detach()，C++标准要求程序
        调用std::terminate()，导致整个程序立即终止。
    */


    //创建独立线程，扫描超时连接
    _scannerThread = ::thread (std::bind(&ConnectionPool::scannerConnectionTask, this));
    _scannerThread.detach();
}

//生产连接函数
void ConnectionPool::produceConnectionTask()
{
    while(_running)
    {
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQue.empty())
        {
            cv.wait(lock);
        }
        if(!_running) 
        {
            LOG_INFO << "线程produce结束";
            break;
        }
        if(_connectionCnt < _maxSize)
        {
            Connection* p = new Connection;
            p->connect(_ip, _port, _usrname, _password, _dbname);
            _connectionQue.push(p);
            p->refreshAliveTime();
            _connectionCnt++;
        }

        cv.notify_all();
    }
}

//获取连接池中的连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while(_connectionQue.empty())
    {
        if(cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeout)) == cv_status::timeout)
        {
            //wait_for:两种情况->1、超时 2、非超时
            //如果没被其他线程唤醒，自己超时醒了
            if(_connectionQue.empty())
            {
                LOG("获取连接超时");
                return nullptr;
            }
        }
    }
    shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon){
                            std::unique_lock<std::mutex> lock(_queueMutex);
                            _connectionQue.push(pcon);
                            pcon->refreshAliveTime();
                             });
    // 当 sp 的引用计数归零时，该 Lambda 会被调用，重新入队
    
    _connectionQue.pop();
    //取完通知生产者线程，检查是否为空
    cv.notify_all();
    return sp;
}

//扫描超时连接
void ConnectionPool::scannerConnectionTask()
{
    while (_running)
    {
       //this_thread::sleep_for(chrono::seconds(_maxIdleTime));
       unique_lock<mutex> lock(_queueMutex);
       cv.wait_for(lock, chrono::seconds(_maxIdleTime));
       if(_running) break;
       while (_connectionCnt > _initSize)
       {
        //入队都是从队尾入队，所以只要队头没超时，后面的连接肯定都不超时
            Connection* p = _connectionQue.front();
            if(p->getAliveTime() >= _maxIdleTime)
            {
                _connectionQue.pop();
                _connectionCnt--;
                delete p;
            }
            else
            {
                break;
            }
       }
        
    }
}