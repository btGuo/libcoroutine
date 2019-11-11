#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#include <list>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "task.h"

using namespace std;

class Scheduler;

class Processor
{
public:
    Processor(Scheduler *scheduler, size_t id);
    void addTask(TaskFn fn, void *args);
    static Processor *& getProcessor();
    list<Task *> moveTask();
    void run();
    size_t taskSize();
    void taskYield();
    void wakeup();
    void showStatistics();

private:
    Task *allocTask();
    void freeTask(Task *task);

    void initContext(Task *task);
    Task *getTask();

    static void taskMain(uint32_t low32, uint32_t high32);


    static thread_local Processor *m_curr;
    static size_t m_ids;
    static atomic<int> m_total_tasks;
    char *        m_stack{nullptr};
    char *        m_stack_top{nullptr};
    size_t        m_stack_size{256*1024};
    size_t        m_task_done{0};
    size_t        m_id;
    ucontext_t    m_ctx;
    list<Task *>  m_free;
    list<Task *>  m_ready;
    mutex         m_ready_lock;
    Scheduler *   m_scheduler{nullptr};
    Task *        m_running_task{nullptr};
    condition_variable m_cv;
};

#endif
