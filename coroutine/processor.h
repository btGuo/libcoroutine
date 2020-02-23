#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#include <list>
#include <unistd.h>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include "../coroutine/workstealingqueue.h"
#include "../coroutine/task.h"
#include "../coroutine/threadsafequeue.h"

namespace co
{

class Scheduler;

class Processor
{
public:
    struct SuspendEntry
    {
        Task *t;
        std::size_t id;
    };
    Processor(Scheduler *scheduler, std::size_t id);
    static Processor *& getThisThreadProcessor();

    SuspendEntry getSuspendEntry();
    /// 开始调度任务
    void run();
    /// 添加任务
    void addTask(Task::TaskFn fn, std::size_t stack_size);
    void addTask(Task *task);
    /// 切出当前任务
    void taskYield();
    /// 阻塞当前任务
    void taskBlock();
    /// 唤醒任务，注意这个是异步调用
    void taskSleep(std::chrono::steady_clock::duration dur);
    void taskWakeup(SuspendEntry entry);
    void showStatistics();
    void runTask(Task *task);
    Task *getRunningTask();

private:

    static thread_local Processor *m_curr; ///< 线程变量，获取单前线程所在的processor
    static std::size_t             m_ids;                   ///< 用户给任务分配id
    static std::atomic<int>        m_total_tasks;      ///< 可运行任务数
    static ThreadSafeQueue<Task *> m_global;           ///< 全局队列
    bool                           m_waiting{false};
    std::size_t                    m_task_count{0};         ///< 本线程任务数
    std::size_t                    m_id;                    ///< 自身id
    ucontext_t                     m_ctx;                   ///< 上下文
    WorkStealingQueue<Task *>      m_ready;                 ///< 就绪任务队列
    std::atomic<std::size_t>       m_blocks{0};             ///< 阻塞任务数量
    std::list<Task *>              m_yield;                 ///< yield任务队列，优先级低
    ThreadSafeQueue<Task *>        m_wakeup;                ///< 待唤醒的任务队列
    Scheduler                     *m_scheduler{nullptr};    ///< 所在调度器
    Task                          *m_running_task{nullptr}; ///< 单前运行的任务
};

}

#endif
