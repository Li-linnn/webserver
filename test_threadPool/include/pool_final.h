#ifndef POOL_FINAL_H
#define POOL_FINAL_H
#include "thread.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "ResType.h"
#include <future>

enum class PoolMode
{
    MODE_FIXED,
    MODE_CATCHED
};
/*
Result 用 std::future， Any用decltype, task用pakaged_task
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
*/

class ThreadPool
{
public:
    ~ThreadPool();
    // submit函数，模版中使用&&，完美转发，左右值参数都可以接受，输入左值调用拷贝构造，输入右值调用移动构造
    template <typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        // Any
        using ResType = decltype(func(args...));
        // 把函数包装成Task对象
        auto task = std::make_shared<std::packaged_task<ResType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    // forward:保持参数的左右值属性，避免被转换成左值而调用拷贝构造，造成不必要的开销
        // Result
        std::future<ResType> result = task->get_future();

        std::unique_lock<std::mutex> lock(TaskThreadMtx_);
        /*
            while(taskNum_ == QueThreshHold_)
            {
                notFull_.wait(lock);
            }
        */
    // notFull_.wait(lock, [&]()->bool{return taskNum_ < QueThreshHold_;});
    // 等待非满通知，阻塞1秒视作任务提交失败
        bool seccess = notFull_.wait_for(lock, std::chrono::seconds(1), 
                            [&]()->bool{ return taskNum_ < QueThreshHold_;});
        if(seccess)
        {
            taskQue_.emplace([=]()->void{ (*task)(); });
            // lambda值捕获task 因此taskQue_中存储lambda-》存储task副本
            taskNum_++;
            // 任务队列非空，通知线程池线程取任务
            notEmpty_.notify_all();
            std::cout << "submit task success!" << std::endl;
    
            if(mode_ == PoolMode::MODE_CATCHED && taskNum_ > idelThread_ && totalThread_ < threadThreshHold_)
            {
                auto thread_ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
                thread_ptr->startThread();
                std::cout << "create new thread" << thread_ptr->getThreadId() << std::endl;
                threads_.emplace(thread_ptr->getThreadId(), std::move(thread_ptr));
                // move以后一定要注意不能再用原指针了！！
                totalThread_++;
                idelThread_++;
            }
            
            return result;// 提交完直接交给线程处理，如何判断哪个是哪个task的返回值
            // return task_ptr->res 在Task中加一个Result成员变量，但线程执行完task->run,task对象直接析构
            // 构造好就返回，不会阻塞其他submit，调用getRes()才阻塞
        }
        else
        {
            auto task = std::packaged_task<ResType()>([]()->ResType{return ResType();});
            std::future<ResType> result = task.get_future();
            std::cerr << "submit task unseccess!" << std::endl;
            return result; // 注册task时就绑定好result对象
        }
    }
    void setQueThreshHold(int threshhold);
    void setPoolMode(PoolMode mode);
    void setThreadThreshHold(int threshhold);
    void threadFunc(int id); // 线程函数
    void startPool(int initNum = 3);
    static ThreadPool* getThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    ThreadPool();
    bool getPoolState() const;

    std::unordered_map<int, std::unique_ptr<Thread>> threads_; // erase指针之后，自动析构Thread对象
    int initThreadNum_; // 初始线程数量
    int threadThreshHold_; // 线程上限阈值
    std::atomic_int idelThread_; // 空闲线程
    std::atomic_int totalThread_; // 当前总线程数量

    using Task = std::function<void()>;
    std::queue<Task> taskQue_;
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