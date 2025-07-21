#include <iostream>
#include "pool_final.h"
#include <chrono>
#include <thread>


int task(int a)
{
    int sum;
    for(int i = 0; i < 100000000; i++)
    {
        sum = sum + i + a;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return sum;
}


int main()
{
    ThreadPool* pool = ThreadPool::getThreadPool();
    pool->setPoolMode(PoolMode::MODE_CATCHED);
    pool->startPool(4);
    auto result1 = pool->submitTask(task, 5);
    std::cout << result1.get() << std::endl;

    auto result2 = pool->submitTask(task, 5);
    auto result3 = pool->submitTask(task, 5);
    auto result4 = pool->submitTask(task, 5);
    auto result5 = pool->submitTask(task, 5);
    std::cout << result5.get() << std::endl;
    auto result6 = pool->submitTask(task, 5);
    std::cout << result6.get() << std::endl;
    auto result7 = pool->submitTask(task, 5);
 
    std::cout << result4.get() << std::endl;
    std::cout << result3.get() << std::endl;
    std::cout << result2.get() << std::endl;
    std::cout << result7.get() << std::endl;

    return 0;
}