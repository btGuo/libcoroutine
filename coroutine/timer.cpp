#include "timer.h"
#include <thread>

namespace co
{

Timer::Timer()
{
    m_precision = std::chrono::milliseconds(1); //1ms
}

Timer::~Timer()
{
}

Timer::TimerEntry *Timer::alloc()
{
    TimerEntry *ret = static_cast<TimerEntry *>(freelistAlloc());
    if(!ret)
        ret = new TimerEntry();
    return ret;
}

void Timer::free(TimerEntry *ptr)
{
    freelistFree(ptr);
}

void Timer::run()
{
    for(;;)
    {
        update();
        std::this_thread::sleep_for(m_precision);
    }
}

void Timer::setPrecision(duration_t dur)
{
    m_precision = dur;
}

int Timer::startTimer(duration_t dur, timerCallBack_t cb)
{
    TimerEntry *entry = alloc();
    entry->canceled = false;
    entry->id = m_ids++;
    entry->cb = cb;
    entry->expired = m_jiffies + dur.count() / m_precision.count();
    m_size++;

    std::lock_guard<std::mutex> lg(m_list_mtx);
    addTimerEntry(entry);
    m_map[entry->id] = entry;

    return entry->id;
} 

void Timer::addTimerEntry(TimerEntry *entry)
{
    std::size_t expires = entry->expired;
    std::size_t idx = expires - m_jiffies;
    List<TimerEntry> *list = nullptr;
    if (idx < TVR_SIZE) // [0, 0x100)
    {     
        int i = expires & TVR_MASK;
        list = &m_near[i];
    }
    else if (idx < (1 << (TVR_BITS + TVN_BITS))) // [0x100, 0x4000)
    {
        int i = (expires >> TVR_BITS) & TVN_MASK;
        list = &m_bucket[0][i];
    }
    else if (idx < (1 << (TVR_BITS + 2 * TVN_BITS))) // [0x4000, 0x100000)
    {
        int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
        list = &m_bucket[1][i];
    }
    else if (idx < (1 << (TVR_BITS + 3 * TVN_BITS))) // [0x100000, 0x4000000)
    {
        int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
        list = &m_bucket[2][i];
    }
    else if ((int64_t)idx < 0)
    {
        // Can happen if you add a timer with expires == jiffies,
        // or you set a timer to go off in the past
        int i = m_jiffies & TVR_MASK;
        list = &m_near[i];
    }
    else
    {
        // If the timeout is larger than MAX_TVAL on 64-bit
        // architectures then we use the maximum timeout
        if (idx > MAX_TVAL)
        {
            idx = MAX_TVAL;
            expires = idx + m_jiffies;
        }
        int i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        list = &m_bucket[3][i];
    }
    // add to linked list
    list->push_back(entry);
}

bool Timer::stopTimer(int id)
{
    auto ret = m_map[id];
    if(ret)
    {
        ret->canceled = true;
        m_map.erase(id);
        m_size--;
        return true;
    }
    return false;
}

bool Timer::cascade(int bucket, int index)
{
    std::lock_guard<std::mutex> lg(m_list_mtx);
    auto list = &m_bucket[bucket][index];
    while(!list->empty())
    {
        auto ret = list->pop_front();
        if(!ret->canceled)
            addTimerEntry(ret);
        else 
            free(ret);
    }
    return index == 0;
}

#define INDEX(N) ((m_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)

int Timer::update()
{
    int fired = execute();
    int index = m_jiffies & TVR_MASK;
    if (index == 0) // cascade timers
    {
        if (cascade(0, INDEX(0)) &&
            cascade(1, INDEX(1)) &&
            cascade(2, INDEX(2)))
            cascade(3, INDEX(3));
    }
#undef INDEX
    
    m_jiffies++;
    fired += execute();
    return fired;
}

int Timer::execute()
{
    std::lock_guard<std::mutex> lg(m_list_mtx);
    int fired = 0;
    int idx = m_jiffies & TVR_MASK;
    List<TimerEntry> *list = &m_near[idx];
    while(!list->empty())
    {
        auto entry = list->pop_front();
        if(!entry->canceled)
        {
            entry->cb();
            m_size--;
            fired++;
            m_map.erase(entry->id);
        }
        free(entry);
    }
    return fired;
}

}//namespace co

