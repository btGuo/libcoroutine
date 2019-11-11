#include "processor.h"
#include "scheduler.h"
#include <chrono>
#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <mutex>

thread_local Processor *Processor::m_curr{nullptr};
size_t Processor::m_ids{0};
atomic<int> Processor::m_total_tasks{0};

mutex outputmtx;

Processor *& Processor::getProcessor()
{
    return m_curr;
}

void Processor::taskMain(uint32_t low32, uint32_t high32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
    Task *task = (Task *)ptr;

    task->fn(task->args);
    task->status = TaskStatus::TaskDead;

    m_total_tasks--;
}

void Processor::taskYield()
{
    char curr;
    Task *task = m_running_task;
    size_t stack_size = (uintptr_t)m_stack_top - (uintptr_t)&curr;

    assert(stack_size < m_stack_size);

    if(task->cap < stack_size)
    {
        task->data = static_cast<char*>(realloc(task->data, stack_size));
        task->cap = stack_size;
    }
    memcpy(task->data, m_stack_top - stack_size, stack_size);
    task->status = TaskStatus::TaskSuspend;

    {
        lock_guard<mutex> lock(m_ready_lock);
        m_ready.push_back(task);
    }
    swapcontext(&task->ctx, &m_ctx);
}

Processor::Processor(Scheduler *scheduler, size_t id)
{
    m_id = id;
    m_scheduler = scheduler;
    m_stack = static_cast<char*>(malloc(m_stack_size));
    m_stack_top = m_stack + m_stack_size;
}

size_t Processor::taskSize()
{
//    lock_guard<mutex> lock(m_ready_lock);
    return m_ready.size();
}

void Processor::addTask(TaskFn fn, void *args)
{
    m_total_tasks++;

    Task *task = allocTask();
    task->fn = fn;
    task->args = args;
    task->status = TaskStatus::TaskReady;
    task->id = m_ids++;

    lock_guard<mutex> lock(m_ready_lock);
    m_ready.push_back(task);
    m_cv.notify_all();
}

Task *Processor::getTask()
{
    unique_lock<mutex> lock(m_ready_lock);
    if(m_ready.empty())
    {
       // cout << "try steal task\n";
        auto ret = m_scheduler->stealTask(this);
        if(!ret.empty())
        {
            //cout << "steal task success" << endl;
            m_ready.merge(ret);
        }
        else // 没有偷到任务，等待
        {
            if(m_total_tasks == 0)
                return nullptr;
            m_cv.wait(lock);
            if(m_total_tasks == 0)
                return nullptr;
        }
    }

    auto ret = m_ready.front();
    m_ready.pop_front();
    return ret;
}

void Processor::initContext(Task *task)
{
    getcontext(&task->ctx);
    task->ctx.uc_stack.ss_sp = m_stack;
    task->ctx.uc_stack.ss_size = m_stack_size;
    task->ctx.uc_link = &m_ctx;

    uintptr_t ptr = (uintptr_t)task;
    makecontext(&task->ctx, (void (*)(void))taskMain, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
} 

void Processor::run()
{
    cout << this_thread::get_id() << " " << __func__ << endl;
    getProcessor() = this;
    while(m_total_tasks)
    {
        Task *task = getTask();
        if(task == nullptr)
        {
            assert(m_total_tasks == 0);
            break;
        }
        //cout << this_thread::get_id() << " " << "get task " << task->id << endl;
        auto status = task->status;
        task->status = TaskStatus::TaskRunning;
        m_running_task = task;

        m_task_done++;

        switch(status){
        case TaskStatus::TaskReady:

            initContext(task);
            swapcontext(&m_ctx, &task->ctx);
            break;

        case TaskStatus::TaskSuspend:

            memcpy(m_stack_top - task->cap, task->data, task->cap);
            swapcontext(&m_ctx, &task->ctx);
            break;

        default:
            cout << "error\n";
            return;
        }
       // cout << "return to run\n";

        if(m_running_task->status == TaskStatus::TaskDead)
        {
            freeTask(m_running_task);
            m_running_task = nullptr;
        }
    }
    assert(m_total_tasks == 0);
    m_scheduler->wakeupAll();    
}

list<Task *> Processor::moveTask()
{
    lock_guard<mutex> lock(m_ready_lock);
    //cout << __func__ << endl;
    list<Task *>ret;
    if(m_ready.size() < 3)
        return ret;

    auto it = m_ready.rbegin();
    int len = m_ready.size() / 3;
    //cout << len << endl;
    while(len--)it++;
    ret.splice(ret.end(), m_ready, it.base(), m_ready.end());

    return ret;
}

Task *Processor::allocTask()
{
    if(m_free.empty())
        return new Task();
    auto ret = m_free.front();
    m_free.pop_front();
    return ret;
}

void Processor::freeTask(Task *task)
{
    m_free.push_back(task);
}

void Processor::wakeup()
{
    m_cv.notify_all();
}

void Processor::showStatistics()
{
    cout << "[" << m_id << "]" << " complete " << m_task_done << " jobs\n";
}

