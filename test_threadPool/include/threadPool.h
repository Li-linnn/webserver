#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "thread.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "ResType.h"

enum class PoolMode
{
    MODE_FIXED,
    MODE_CATCHED
};

class Result;
class Task
{
public:
    Task() = default;
    ~Task() = default;
    virtual Any run() = 0;
    void exec();
    void setResult(Result* res);// 给task绑定Result对象
private:
    Result* res_;
};

// 线程通信信号量
class Semaphore
{
public:
    Semaphore(int resLimit = 0);
    ~Semaphore() = default;
    void wait();
    void post();
private:
    int resLimit_;
    std::mutex sem_lock;
    std::condition_variable cv;
};

class Result
{
public:
    Result(std::shared_ptr<Task> task_ptr = nullptr, bool isSuccess = true);
    ~Result() = default;
    Any getRes(); // 用户调用
    void setRes(Any res); // task调用
private:
    Any res_;
    std::shared_ptr<Task> task;
    Semaphore sem;
    // bool success;
    std::atomic_bool success;
};

class ThreadPool
{
public:
    ~ThreadPool();

    Result submitTask(std::shared_ptr<Task> task);// 给线程池提交任务,指针实现多态
    void setQueThreshHold(int threshhold);
    void setPoolMode(PoolMode mode);
    void setThreadThreshHold(int threshhold);
    void threadFunc(int id);// 线程函数
    void startPool(int initNum = 3);
    static ThreadPool* getThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    ThreadPool();
    bool getPoolState() const;

    std::unordered_map<int, std::unique_ptr<Thread>> threads_;// erase指针之后，自动析构Thread对象
    int initThreadNum_; // 初始线程数量
    int threadThreshHold_; // 线程上限阈值
    std::atomic_int idelThread_; // 空闲线程
    std::atomic_int totalThread_; // 当前总线程数量

    std::queue<std::shared_ptr<Task>> taskQue_;
    std::atomic_int taskNum_; // 任务数量
    int QueThreshHold_; // 任务队列上限阈值

    std::mutex TaskThreadMtx_; 
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::condition_variable exitCond_;

    PoolMode mode_;
    std::atomic_bool poolIsRunning_;
};

#endif