#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadCount)
    : stop(false),
      activeTasks(0)
{
    if (threadCount == 0)
    {
        threadCount = 1;
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        workers.emplace_back([this]()
        {
            while (true)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(mutex);

                    taskCondition.wait(lock, [this]()
                    {
                        return stop || !tasks.empty();
                    });

                    if (stop && tasks.empty())
                    {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();

                {
                    std::unique_lock<std::mutex> lock(mutex);

                    --activeTasks;

                    if (activeTasks == 0 && tasks.empty())
                    {
                        finishedCondition.notify_all();
                    }
                }
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    wait();

    {
        std::unique_lock<std::mutex> lock(mutex);
        stop = true;
    }

    taskCondition.notify_all();

    for (std::thread& worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::submit(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(mutex);

        tasks.push(std::move(task));
        ++activeTasks;
    }

    taskCondition.notify_one();
}

void ThreadPool::wait()
{
    std::unique_lock<std::mutex> lock(mutex);

    finishedCondition.wait(lock, [this]()
    {
        return activeTasks == 0 && tasks.empty();
    });
}