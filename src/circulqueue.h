#ifndef __CIRCUL_QUEUE_H
#define __CIRCUL_QUEUE_H

#include <assert.h>
#include <atomic>

/**
 * 环形队列
 */
template <typename T>
class CirculQueue
{
public:
    CirculQueue(size_t logcap = 16):m_log(logcap),m_size(1UL << logcap),m_mask(m_size - 1UL)
    {
        assert(m_log < 32);
        m_data = new std:: atomic<T>[m_size];
    }
    ~CirculQueue(){ delete [] m_data; }
    size_t size(){  return m_size; }
    T get(size_t i){ return m_data[i & m_mask]; }
    void put(size_t i, T elem){ m_data[i & m_mask].store(elem); }
    std:: shared_ptr<CirculQueue<T>> new_double_capacity()
    {
        std:: shared_ptr<CirculQueue<T>> new_queue = 
            std:: make_shared<CirculQueue<T>>(m_log + 1);
        for(size_t i = 0; i < m_size; i++)
            new_queue->put(i, m_data[i]);

        return new_queue;
    }
    
private:
    const size_t m_log;
    const size_t m_size;
    const size_t m_mask;
    std:: atomic<T> *m_data{nullptr};
};

#endif
