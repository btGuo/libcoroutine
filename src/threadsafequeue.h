#ifndef __MUTEX_QUEUE_H
#define __MUTEX_QUEUE_H

#include <list>
#include <mutex>
#include <condition_variable>
#include "common.h"

namespace co
{
/**
 * 线程安全队列
 * @tparam T 元素类型
 * @tparam ContainerType 底层容器类型，默认为stl的list
 * @tparam LockType 锁类型，默认为mutex
 */
template <typename T, 
         typename ContainerType = std::list<T>, 
         typename LockType = std::mutex>
class ThreadSafeQueue
{
public:
    template <typename U, 
             enable_if_t<std::is_same<T, remove_reference_t<U> >::value, int> = 0>
    void push(U &&elem)
    {
        std::lock_guard<LockType> lg(m_lock);
        m_queue.push_back(std::forward<U>(elem));
        m_cv.notify_one();
    }
    void waitAndPop(T &elem)
    {
        std::unique_lock<LockType> lg(m_lock);
        m_cv.wait(lg, [this]{ return !m_queue.empty(); });
        elem = std::move(m_queue.front());
        m_queue.pop_front();
    }
    bool pop(T &elem)
    {
        std::lock_guard<LockType> lg(m_lock);
        if(m_queue.empty())
            return false;
        elem = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }
    size_t size()
    {
        std::lock_guard<LockType> lg(m_lock);
        return m_queue.size();
    }
private:
    ContainerType           m_queue;
    LockType                m_lock;
    std::condition_variable m_cv;
};

}

#endif
