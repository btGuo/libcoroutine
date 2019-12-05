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
list<Task *> Processor:: m_global;
mutex Processor:: m_global_mutex;

Processor::Processor(Scheduler *scheduler, size_t id)
{
    m_id = id;
    m_scheduler = scheduler;
}

Processor *& Processor::getThisThreadProcessor()
{
    return m_curr;
}

void Processor::taskYield()
{
    Task *task = m_running_task;
    m_ready.push(task);
    task->yield(&m_ctx);
}

void Processor::addTask(Task:: TaskFn fn, size_t stack_size)
{
    m_task_count++;
    m_total_tasks++;
    Task *task = new Task(fn, m_ids++, stack_size);

    if(m_task_count % 32)
    {
        lock_guard<mutex> lock(m_global_mutex);
        m_global.push_back(task);
        m_cv.notify_one();
    }
    else 
    {
        m_ready.push(task);
    }
}

void Processor::runTask(Task *task)
{
    //cout << "[" << m_id << "] " << "get task " << task->id << endl;
    m_running_task = task;

    task->swapIn(&m_ctx);

    if(task->taskDead())
    {
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

    while(m_total_tasks)
    {
        Task *task;
        if(m_ready.pop(task))
        {
            runTask(task);
            continue;
        }

        //先去全局队列里面找
        {
            lock_guard<mutex> lock(m_global_mutex);
            if(!m_global.empty())
            {
                m_ready.push(m_global.front());
                m_global.pop_front();
                continue;
            }
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

        if(m_ready.size() == 0)
        {
            unique_lock<mutex> lock(m_global_mutex);
            m_cv.wait(lock);
        }
    }

    static bool mark = false;
    if(mark) return;
    mark = true;

    assert(m_total_tasks == 0);
    //cout << "done\n";
    m_scheduler->wakeupAll();    
}

void Processor::wakeup()
{
    m_cv.notify_all();
}

void Processor::showStatistics()
{
    cout << "[" << m_id << "]" << " complete " << m_task_count << " jobs\n";
}

}
