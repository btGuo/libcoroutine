#ifndef __WORK_STEALING_QUEUE_H
#define __WORK_STEALING_QUEUE_H

#include <assert.h>
#include <atomic>

namespace co
{

/**
 * 环形队列，T只能是基本数据类型
 */
template <typename T>
class CirculQueue
{
public:
    CirculQueue(size_t logcap = 16):m_log(logcap),m_size(1UL << logcap),m_mask(m_size - 1UL)
    {
        assert(m_log < 32);
        m_data = new std::atomic<T>[m_size];
    }
    ~CirculQueue(){ delete [] m_data; }
    size_t size(){  return m_size; }
    T get(size_t i){ return m_data[i & m_mask].load(); }
    void put(size_t i, T elem){ m_data[i & m_mask].store(elem); }
    std::shared_ptr<CirculQueue<T>> new_double_capacity()
    {
        std::shared_ptr<CirculQueue<T>> new_queue = 
            std::make_shared<CirculQueue<T>>(m_log + 1);
        for(size_t i = 0; i < m_size; i++)
            new_queue->put(i, m_data[i]);

        return new_queue;
    }
    
private:
    const size_t     m_log;
    const size_t     m_size;
    const size_t     m_mask;
    std::atomic<T>  *m_data{nullptr};
};

/**
 * 无锁任务窃取队列，根据《多处理器编程的艺术》16章算法写的
 * @note push和pop操作同时只能有一个，但和steal操作可以并发，多个steal可以并发
 */
template <typename T>
class WorkStealingQueue
{
public:
    WorkStealingQueue()
    {
        m_tasks = std::make_shared<CirculQueue<T>>();
    }

    std::size_t size()
    {
        std::size_t bottom = m_bottom.load();
        std::size_t top = m_top.load();
        return bottom > top ? bottom - top : 0;
    }

    bool empty()
    {
        std::size_t top = m_top.load();
        std::size_t bottom = m_bottom.load();
        return bottom <= top;
    }
    
    void clear()
    {
        m_bottom = 1;
        m_top = 1;
    }

    /**
     * 添加任务
     */
    void push(const T item)
    {
        std::size_t oldbottom = m_bottom.load();
        std::size_t oldtop = m_top.load();
        std::shared_ptr<CirculQueue<T>> curr_m_tasks = m_tasks;

        if(oldbottom > oldtop && // 先判断是否大于，因为是无符号的，不能直接相减 
           oldbottom - oldtop >= m_tasks->size() - 1)
        {
            curr_m_tasks = curr_m_tasks->new_double_capacity();
            m_tasks = curr_m_tasks;
        }
        m_tasks->put(oldbottom, item);
        m_bottom++;
    }

    /**
     * 取出任务
     * @param result 取出来的结果
     * @reture 是否成功
     */
    bool pop(T &result)
    {
        m_bottom--;
        std::size_t oldtop = m_top.load();
        std::size_t newtop = oldtop + 1;
        if(m_bottom < oldtop)
        {
            m_bottom.store(oldtop);
            return false;
        }

        auto task = m_tasks->get(m_bottom.load());
        if(m_bottom > oldtop)
        {
            result = task;
            return true;
        }

        if(!m_top.compare_exchange_strong(oldtop, newtop))
            return false;

        m_bottom.store(newtop);
        result = task;
        return true;
    }

    /**
     * 偷取任务
     * @param result 取出来的结果
     * @reture 是否成功
     */
    bool steal(T &result)
    {
        std::size_t oldtop = m_top.load();
        std::size_t newtop = oldtop + 1;
        std::size_t oldbottom = m_bottom.load();

        if(oldbottom <= oldtop)
            return false;
        auto task = m_tasks->get(oldtop);
        if(!m_top.compare_exchange_strong(oldtop, newtop))
            return false;

        result = task;
        return true;
    }

private:
    std::atomic<std::size_t>        m_bottom{1};
    std::atomic<std::size_t>        m_top{1};
    std::shared_ptr<CirculQueue<T>> m_tasks;
};

}

#endif
