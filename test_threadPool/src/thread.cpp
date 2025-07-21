#include "thread.h"
#include <iostream>
#include <thread>

int Thread::generateId = 0;

Thread::Thread(ThreadFunc func)
            :func_(func), threadId_(generateId++)
{}

void Thread::startThread()
{
    std::thread t(func_, threadId_); 
    t.detach();
}

int Thread::getThreadId() const
{
    return threadId_;
}
Thread::~Thread()
{

}