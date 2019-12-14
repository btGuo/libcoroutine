#include "processor.h"
#include "scheduler.h"
#include <chrono>
#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <mutex>
#include <random>

namespace co
{

using namespace std;

thread_local Processor *Processor::m_curr{nullptr};
size_t Processor::m_ids{0};
atomic<int> Processor::m_total_tasks{0};
ThreadSafeQueue<Task *> Processor::m_global;

Processor::Processor(Scheduler *scheduler, size_t id)
{
    m_id = id;
    m_scheduler = scheduler;
}

Processor *& Processor::getThisThreadProcessor()
{
    return m_curr;
}

void Processor::taskBlock()
{
    //cout << "task " << m_running_task->getId() << " block" << endl;
    m_blocks++;
    m_running_task->block(&m_ctx);
}

void Processor::taskYield()
{
    Task *task = m_running_task;
    m_yield.push_back(task);
    task->yield(&m_ctx);
}

void Processor::taskWakeup(Task *task)
{
    m_wakeup.push(task);
    // 顺序不能反!!!
    m_blocks--;
    //cout << "task " << task->getId() << " wakeup" << endl;
}

void Processor::addTask(Task *task)
{
    m_task_count++;
    m_total_tasks++;
    if(m_task_count % 32 == 0)
    {
        m_global.push(task);
    }
    else 
    {
        m_ready.push(task);
    }
}

void Processor::addTask(Task::TaskFn fn, size_t stack_size)
{
    addTask(new Task(fn, m_ids++, stack_size));
}

void Processor::runTask(Task *task)
{
    //cout << "[" << m_id << "] " << "get task " << task->getId() << endl;
    m_running_task = task;

    task->swapIn(&m_ctx);

    if(task->taskDead())
    {
        //cout << "[" << task->getId() << "] " << "work done" << endl;
        delete task;
        m_running_task = nullptr;
        m_total_tasks--;
    }
}

void Processor::run()
{
    //cout << m_id << " " << __func__ << endl;
    m_curr = this;
    random_device rd;
    default_random_engine random(rd());

    while(true)
    {
        Task *task;
        if(m_wakeup.pop(task) || m_ready.pop(task))
        {
            runTask(task);
            continue;
        }

        //先拿出yield任务
        if(!m_yield.empty())
        {
            m_ready.push(m_yield.front());
            m_yield.pop_front();
            continue;
        }

        //尝试窃取任务
        size_t procsize = m_scheduler->getProcessorSize();
        procsize <<= 1;
        while(procsize--)
        {
            Processor *victim = m_scheduler->getProcessor(
                    random() % m_scheduler->getProcessorSize());
            if(victim == this)
                continue;
    
            if(victim->m_ready.size())
            {
                size_t quesize = victim->m_ready.size();
                auto & stealque = victim->m_ready;
                quesize >>= 1;
                while(quesize--)
                {
                    Task *ret;
                    if(stealque.steal(ret))
                        m_ready.push(ret);
                }
                break;
            }
        }
        //这个判读一定要加
        if(m_ready.size())
            continue;

        //在全局队列里面找
        if(m_global.pop(task))
        {
            m_ready.push(task);
            continue;
        }

        //是否有阻塞任务，等待
        if(m_blocks.load())
        {
            //cout << "processor " << m_id << " wait m_wakeup" << endl;
            m_wakeup.waitAndPop(task);
            m_ready.push(task);
            continue;
        }
        
        //没有阻塞任务，在全局队列上等待
        //cout << "processor " << m_id << " wait m_global" << endl;
        m_global.waitAndPop(task);
        m_ready.push(task);
    }
}

void Processor::showStatistics()
{
    cout << "[" << m_id << "]" << " complete " << m_task_count << " jobs\n";
}

Task *Processor::getRunningTask() 
{
    return m_running_task;
}

}
