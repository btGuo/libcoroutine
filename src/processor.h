#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#include <list>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "workstealingqueue.h"
#include "task.h"

namespace co
{

class Scheduler;

class Processor
{
public:
    Processor(Scheduler *scheduler, std:: size_t id);
    static Processor *& getThisThreadProcessor();

    /// 开始调度任务
    void run();
    /// 添加任务
    void addTask(Task:: TaskFn fn, std:: size_t stack_size);
    /// 任务切出
    void taskYield();
    /// 唤醒自身
    void wakeup();
    void showStatistics();
    void runTask(Task *task);

private:

    static thread_local Processor *m_curr; ///< 线程变量，获取单前线程所在的processor
    static std::size_t m_ids;                   ///< 用户给任务分配id
    static std::atomic<int> m_total_tasks;      ///< 可运行任务数

    bool                m_waiting{false};
    std:: size_t        m_task_count{0};          ///< 本线程任务数
    std:: size_t        m_id;                    ///< 自身id
    ucontext_t          m_ctx;                   ///< 上下文
    WorkStealingQueue<Task *> m_ready;         
    static std:: list<Task *> m_global;
    static std:: mutex        m_global_mutex; 

    Scheduler *   m_scheduler{nullptr};    ///< 所在调度器
    Task *        m_running_task{nullptr}; ///< 单前运行的任务
    std::condition_variable m_cv;              
};

}

#endif
