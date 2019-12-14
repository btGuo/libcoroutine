#include "scheduler.h"
#include <iostream>
#include <chrono>
#include <random>

namespace co
{
using namespace std;

size_t Scheduler::m_ids{0};

void Scheduler::start(int min, int max)
{
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

Scheduler::Scheduler()
{
    m_self = new Processor(this, m_ids++);
    m_self->getThisThreadProcessor() = m_self;
    m_processors.push_back(m_self);
}

Scheduler::~Scheduler()
{
    for(auto &p:m_processors)
        delete p;
}
}
