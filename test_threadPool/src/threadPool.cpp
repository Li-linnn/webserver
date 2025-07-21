#include "threadPool.h"
#include <functional>
#include <thread>
#include <iostream>

constexpr int MAX_QUE_THRESHHLOD = 1024;
constexpr int MAX_THREAD_THRESHHOLD = 10;
constexpr int MAX_THREAD_IDEL_TIME = 10;

void Task::exec()
{
    res_->setRes(run());
}
void Task::setResult(Result* res) // 绑定Result对象
{
    res_ = res;
}

Result::Result(std::shared_ptr<Task> task_ptr, bool isSuccess)
    :task(task_ptr),
    success(isSuccess)
{
    task->setResult(this);
}

Any Result::getRes() // 用户调用
{
    if(!success)
    {
        return false;
    }
    else
    {
        sem.wait();// task未执行完时，用户线程阻塞
        return std::move(res_);
    }
}

void Result::setRes(Any res) // task调用
{
    res_ = std::move(res);
    sem.post();
}

Semaphore::Semaphore(int resLimit)
    :resLimit_(resLimit)
{}
void Semaphore::wait()
{
    std::unique_lock<std::mutex> lock(sem_lock);
    cv.wait(lock, [&]()->bool{return resLimit_ > 0;});
    resLimit_--;
}
void Semaphore::post()
{
    std::unique_lock<std::mutex> lock(sem_lock);
    resLimit_++;
    cv.notify_all();
}

ThreadPool::ThreadPool()
    :initThreadNum_(0), 
    threadThreshHold_(MAX_THREAD_THRESHHOLD),
    QueThreshHold_(MAX_QUE_THRESHHLOD), 
    taskNum_(0),
    poolIsRunning_(false),
    mode_(PoolMode::MODE_FIXED)
{}
ThreadPool::~ThreadPool()
{
    poolIsRunning_ = false;

    std::unique_lock<std::mutex> lock(TaskThreadMtx_);
    notEmpty_.notify_all();
    exitCond_.wait(lock, [&]()->bool{return threads_.empty();});
}

Result ThreadPool::submitTask(std::shared_ptr<Task> task_ptr)
{
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
        taskQue_.push(task_ptr);
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

        
        return Result(task_ptr, true);// 提交完直接交给线程处理，如何判断哪个是哪个task的返回值
        // return task_ptr->res 在Task中加一个Result成员变量，但线程执行完task->run,task对象直接析构
        // 构造好就返回，不会阻塞其他submit，调用getRes()才阻塞
    }
    else
    {
        std::cerr << "submit task unseccess!" << std::endl;
        return Result(task_ptr, false);// 注册task时就绑定好result对象
    }
}

// 设置任务队列上限阈值
void ThreadPool::setQueThreshHold(int threshhold)
{
    if(getPoolState())
    {
        return;
    }
    QueThreshHold_ = threshhold;
}

// 设置线程池模式
void ThreadPool::setPoolMode(PoolMode mode)
{
    if(getPoolState())
    {
        return;
    }
    mode_ = mode;
}

void ThreadPool::setThreadThreshHold(int threshhold)
{
    if(getPoolState())
    {
        return;
    }
    if(mode_ == PoolMode::MODE_CATCHED)
    {
        threadThreshHold_ = threshhold;
    }
}

bool ThreadPool::getPoolState() const
{
    return poolIsRunning_;
}

// 线程函数
void ThreadPool::threadFunc(int threadId)
{
    while(poolIsRunning_)
    {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(TaskThreadMtx_);
            std::cout << "thread: " << threadId << " request task" << std::endl;
            // 死锁：pool析构时有刚进来要获取锁的线程
            if(!poolIsRunning_ && taskQue_.empty())
            {
                break;
            }
            // 等待非空条件
            if(mode_ == PoolMode::MODE_CATCHED)
            {
                while(taskQue_.empty())
                {
                    if(notEmpty_.wait_for(lock, std::chrono::seconds(20)) == std::cv_status::timeout
                    && totalThread_ > initThreadNum_)
                    {
                        threads_.erase(threadId);
                        totalThread_--;
                        std::cout << "thread: " << threadId << " exit!" << std::endl;
                        return;
                    }
                }
            }
            else
            {
                notEmpty_.wait(lock, [&]()->bool{return !taskQue_.empty();});
            }
            // pool析构时正阻塞在wait的线程，检查是否被 线程池关闭信号唤醒
            if(!poolIsRunning_ && taskQue_.empty())
            {
                break;
            }

            task = taskQue_.front();
            taskQue_.pop();
            taskNum_--;

            std::cout << "thread: " << threadId << " acquire task" << std::endl;

            notEmpty_.notify_all();
            notFull_.notify_all();
        }
        // 取完任务及时释放锁
        if(task != nullptr)
        {
            idelThread_--;
            // return task->run();
            task->exec();
        }
        // pool析构时在执行task的线程，执行完跳出while
    }
    {
        std::unique_lock<std::mutex> lock(TaskThreadMtx_);
        threads_.erase(threadId);
        totalThread_--;
        std::cout << "thread: " << threadId << " exit!" << std::endl;
        exitCond_.notify_all();
    }
}

// 启动线程池
void ThreadPool::startPool(int initNum)
{
    initThreadNum_ = initNum;
    poolIsRunning_ = true;

    for(int i = 0; i < initThreadNum_; i++)
    {
        auto thread_ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        threads_.emplace(thread_ptr->getThreadId(), std::move(thread_ptr));
        totalThread_++;
       // threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this)));
    }

    for(auto& thread_ : threads_)
    {
        thread_.second->startThread();
        std::cout << "create thread: " << thread_.second->getThreadId() << std::endl;
        idelThread_++;
    }
}

ThreadPool* ThreadPool::getThreadPool()
{
    static ThreadPool pool;
    return &pool;
}

// Result ThreadPool::submit(std::function<Any()>)