#ifndef __CHANNEL_H
#define __CHANNEL_H

#include <list>
#include <mutex>
#include <utility>
#include "task.h"
#include "processor.h"
#include "common.h"

namespace co
{
template <typename T>
class Channel 
{
public:
    Channel(std::size_t buflen = 0):m_buflen(buflen){}

    template <typename U, 
             enable_if_t<std::is_same<T, remove_reference_t<U> >::value, int> = 0>
    Channel & operator << (U &&item)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_queue.push_back(std::forward<U>(item));
        if(!m_rque.empty())
        {
            Task *t = m_rque.front();
            m_rque.pop_front();
            t->getProcessor()->taskWakeup(t);
        }
            
        //这里用if，不是while
        if(m_queue.size() > m_buflen)
        {
            Processor *p = Processor::getThisThreadProcessor();
            m_wque.push_back(p->getRunningTask());
            lock.unlock();
            p->taskBlock();
            lock.lock();
        }
        return *this;
    }

    Channel & operator >> (T &item)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        //可能有假唤醒，用while
        while(m_queue.empty())
        {
            Processor *p = Processor::getThisThreadProcessor();
            m_rque.push_back(p->getRunningTask());
            lock.unlock();
            p->taskBlock();
            lock.lock();
        }
        item = m_queue.front();
        m_queue.pop_front();

        if(!m_wque.empty())
        {
            Task *t = m_wque.front();
            m_wque.pop_front();
            t->getProcessor()->taskWakeup(t);
        }
        return *this;
    }

    Channel(Channel &ch) = delete;
    Channel(Channel &&ch) = delete;
    Channel & operator = (Channel &ch) = delete;
    Channel & operator = (Channel &&ch) = delete;

private:
    std::size_t       m_buflen{0};
    std::list<T>      m_queue;
    std::list<Task *> m_wque;
    std::list<Task *> m_rque;
    std::mutex        m_mtx;
};

}

#endif
