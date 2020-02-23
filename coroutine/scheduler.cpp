#include "scheduler.h"
#include "common.h"
#include "timer.h"
#include "taskstackallocator.h"
#include <iostream>
#include <chrono>
#include <random>

namespace co
{
using namespace std;

void Scheduler::start(int min, int max)
{
    m_start = true;
    if(min == 0)
        min = thread::hardware_concurrency();

    if(max == 0 || max < min)
        max = min;

    m_min_threads = min;
    m_max_threads = max;

    for(int i = 0; i < min - 1; i++)
    {
        auto p = new Processor(this, m_ids++);
        m_processors.push_back(p);
        thread t([p]{ 
            p->run();
        });
        t.detach();
    }      
    m_self->run();
    showStatistics();
}

Timer *Scheduler::getTimer()
{
    static ThreadHelper thh([this]{
        this->m_timer->run();
    });
        
    return m_timer;
}

size_t Scheduler::getProcessorSize()
{
    return m_processors.size();
}

Processor *Scheduler::getProcessor(size_t id)
{
    return m_processors[id];
}

void Scheduler::showStatistics()
{
    cout << "statics\n";
    cout << "===========================\n";
    for(auto p : m_processors)
        p->showStatistics();

    cout << "===========================\n";
}

bool Scheduler::running()
{
    return m_start;
}

Scheduler::Scheduler()
{
    m_timer = new Timer();
    m_self = new Processor(this, m_ids++);
    m_self->getThisThreadProcessor() = m_self;
    m_processors.push_back(m_self);
}

Scheduler::~Scheduler()
{
    for(auto &p:m_processors)
        delete p;
    if(m_timer)
        delete m_timer;
}

void Scheduler::setStackSize(size_t stack_size)
{
    TaskStackAllocator::getInstance().setSize(stack_size);
}

std::size_t Scheduler::getStackSize() 
{
    return TaskStackAllocator::getInstance().getSize();
}

void Scheduler::create(Task::TaskFn fn,
          size_t stack_size = TaskStackAllocator::getInstance().getSize())
{
    Processor::getThisThreadProcessor()->addTask(fn, stack_size);
}

void Scheduler::yield()
{
    Processor::getThisThreadProcessor()->taskYield();
}

void Scheduler::sleep(std::chrono::steady_clock::duration dur)
{
    Processor::getThisThreadProcessor()->taskSleep(dur);
}

}
