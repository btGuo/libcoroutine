#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include <list>
#include <vector>
#include "processor.h"

using namespace std;

class Scheduler
{
public:
    /**
     * 启动调度器
     * @param min: 最小调度线程数，为0时，设置为cpu核心数，
     * @param max: 最大调度线程数，为0时，设置为min，
     * @note 如果max大于min，有必要时会自动扩充调度线程数.
     */
    void start(int min = 1, int max = 0);
    list<Task *> stealTask(Processor *p);
    void addTask(TaskFn fn, void *args);
    static Scheduler &getInstance();
    void wakeupAll();
private:

    vector<Processor *> m_processors;
    Processor *m_self{nullptr};
    int m_min_threads{0};
    int m_max_threads{0};
    static size_t m_ids;

    Scheduler();
    ~Scheduler();
    Scheduler(Scheduler const&) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler& operator=(Scheduler const&) = delete;
    Scheduler& operator=(Scheduler &&) = delete;
};

#endif
