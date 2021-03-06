#ifndef __COROUTINE_H
#define __COROUTINE_H

#include "scheduler.h"
#include "channel.h"
#include <functional>
#include <chrono>

template <typename T>
using co_chan = co::Channel<T>;

extern bool hook_enable;

static inline void 
co_create(co::Task::TaskFn fn, 
          size_t stack_size = co::Scheduler::getInstance().getStackSize())
{
    co::Scheduler::getInstance().addTask(fn, stack_size);
}

static inline void 
co_setstacksize(size_t stack_size)
{
    co::Scheduler::getInstance().setStackSize(stack_size);
}

static inline std::size_t co_getstacksize()
{
    return co::Scheduler::getInstance().getStackSize();
}

static inline void co_yield()
{
    co::Scheduler::getInstance().yield();
}

static inline co::Scheduler &co_sched()
{
    return co::Scheduler::getInstance();
}

static inline void co_hook_enable()
{
    hook_enable = true;
}

static inline void co_hook_disable()
{
    hook_enable = false;
}

static inline void 
co_sleep(std::chrono::steady_clock::duration dur)
{
    co::Scheduler::getInstance().sleep(dur);
}

#endif
