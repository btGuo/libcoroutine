// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.
//
// 改自https://github.com/ichenq/timer-benchmarks
#ifndef __TIMER_H
#define __TIMER_H

#include "freelist.h"
#include "list.h"
#include <chrono>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <functional>

namespace co
{
class Timer:public FreeList
{
public:
    using timerCallBack_t = std::function<void()>;
    using time_point_t    = std::chrono::steady_clock::time_point;
    using duration_t      = std::chrono::steady_clock::duration;
    struct TimerEntry:public ListHook<TimerEntry>
    {
        bool canceled{false};
        int id{-1};
        std::size_t expired;
        timerCallBack_t cb;
    };
    Timer();
    ~Timer();
    void run();
    void setPrecision(duration_t dur);
    int startTimer(duration_t dur, timerCallBack_t cb);
    bool stopTimer(int id);
    int update();
private:
    enum
    {
        WHEEL_BUCKETS = 4,
        TVN_BITS = 6,                   // time vector level shift bits
        TVR_BITS = 8,                   // timer vector shift bits
        TVN_SIZE = (1 << TVN_BITS),     // wheel slots of level vector
        TVR_SIZE = (1 << TVR_BITS),     // wheel slots of vector
        TVN_MASK = (TVN_SIZE - 1),      //
        TVR_MASK = (TVR_SIZE - 1),      //
        MAX_TVAL = ((uint64_t)((1ULL << (TVR_BITS + 4 * TVN_BITS)) - 1)),
    };

    void addTimerEntry(TimerEntry *entry);
    TimerEntry *alloc();
    void free(TimerEntry *ptr);
    int execute();
    bool cascade(int bucket, int index);
    
    duration_t m_precision;
    std::atomic<std::size_t> m_size{0};
    std::size_t m_jiffies{0};
    List<TimerEntry> m_near[TVR_SIZE];
    List<TimerEntry> m_bucket[WHEEL_BUCKETS][TVN_SIZE];
    std::mutex m_list_mtx;
    std::mutex m_map_mtx;
    std::unordered_map<int, TimerEntry *>m_map;
    std::atomic<int> m_ids{0};
};

}//namespace co

#endif
