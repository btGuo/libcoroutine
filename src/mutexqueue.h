#ifndef __THREAD_SAFE_QUEUE_H
#define __THREAD_SAFE_QUEUE_H

#include <list>
#include <mutex>

namespace co
{

template <typename T, 
         typename ContainerType = std:: list<T>, 
         typename LockType = std:: mutex>
class ThreadSafeQueue
{
public:
    void push(const T &elem)
    {
        std:: lock_guard<LockType> lg(m_lock);
        m_queue.push_back(std:: forward(elem));
    }
    void pop(T &elem)
    {
        std:: lock_guard<LockType> lg(m_lock);
        elem = std:: move(m_queue.front());
        m_queue.pop_front();
    }
    bool empty()const
    {
        std:: lock_guard<LockType> lg(m_lock);
        return m_queue.empty();
    }
    size_t size()const
    {
        std:: lock_guard<LockType> lg(m_lock);
        return m_queue.size();
    }
private:
    ContainerType    m_queue;
    mutable LockType m_lock;
};

}

#endif
