#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 异步写日志的队列
template<class T>
class LockQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
public:
    //多个线程写queue
    void Push(const T& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();
    }

    //一个线程读取queue
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            //日志队列空，线程等待
            m_condvariable.wait(lock);
        }

        T data=m_queue.front();
        m_queue.pop();
        return data;
    }
};
