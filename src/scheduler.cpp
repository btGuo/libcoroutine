#include "scheduler.h"
#include <iostream>
#include <chrono>
#include <random>

size_t Scheduler::m_ids{0};

list<Task *> Scheduler::stealTask(Processor *_p)
{
    list<Task *>ret;
    random_device rd;
    mt19937 gen(rd());

    Processor *p = nullptr;
    size_t len = m_processors.size();
    uniform_int_distribution<> dis(0, len - 1);

    // 随机遍历len 次
    while(len--)
    {
        p = m_processors[dis(gen)];
        if(p == _p)
            continue;

        ret = p->moveTask();
        if(!ret.empty())
            return ret;
    }

    // 还没找到，则顺序遍历
    for(auto p:m_processors)
    {
        if(p == _p)
            continue;

        ret = p->moveTask();
        if(!ret.empty())
            return ret;
    }
    return ret;
}

Scheduler::Scheduler()
{
    m_self = new Processor(this, m_ids++);
    m_processors.push_back(m_self);
}

Scheduler::~Scheduler()
{
    for(auto &p:m_processors)
        delete p;
}

Scheduler &Scheduler::getInstance()
{
    static Scheduler self;
    return self;
}

void Scheduler::addTask(TaskFn fn, void *args)
{
    // 随机加入任务
    random_device rd;
    mt19937 gen(rd());

    size_t len = m_processors.size();
    uniform_int_distribution<> dis(0, len - 1);
    Processor *p = m_processors[dis(gen)];

    p->addTask(fn, args);
}

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

void Scheduler::showStatistics()
{
    cout << "statics\n";
    cout << "===========================\n";
    for(auto p : m_processors)
        p->showStatistics();

    cout << "===========================\n";
}

void Scheduler::wakeupAll()
{
    for(auto p : m_processors)
        p->wakeup();
}
