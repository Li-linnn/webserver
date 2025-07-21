#ifndef THREAD_H
#define THREAD_H

#include <iostream>
#include <functional>
#include <atomic>

class Thread
{
public:
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc func);
    ~Thread();

    void startThread();
    int getThreadId() const;
private:
    // 线程函数
    ThreadFunc func_;
    static int generateId;
    int threadId_;
};

#endif