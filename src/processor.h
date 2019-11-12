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
    static Processor *& getProcessor();

    /// 拿出自身一部分任务给别的processor
    list<Task *> moveTask();
    /// 开始调度任务
    void run();
    /// 添加任务
    void addTask(TaskFn fn, void *args);
    /// 任务切出
    void taskYield();
    /// 唤醒自身
    void wakeup();
    void showStatistics();

private:
    /// 分配任务
    Task *allocTask();
    /// 释放任务
    void freeTask(Task *task);

    /// 初始化任务上下文
    void initContext(Task *task);
    /**
     * 从任务队列获取任务
     * @note 如果本地队列没有任务，则会去偷取任务，如果没偷到，则阻塞等待新任务到来
     */
    Task *getTask();

    static void taskMain(uint32_t low32, uint32_t high32);

    static thread_local Processor *m_curr; ///< 线程变量，获取单前线程所在的processor
    static size_t m_ids;                   ///< 用户给任务分配id
    static atomic<int> m_total_tasks;      ///< 可运行任务数

    char *        m_stack{nullptr};        ///< 栈
    char *        m_stack_top{nullptr};    ///< 栈顶，= m_stack + m_stack_size
    size_t        m_stack_size{256*1024};  ///< 栈大小，这里是256K
    size_t        m_task_done{0};          ///< 完成任务数
    size_t        m_id;                    ///< 自身id
    ucontext_t    m_ctx;                   ///< 上下文
    list<Task *>  m_free;                  ///< 回收任务
    list<Task *>  m_ready;                 ///< 可运行任务队列
    mutex         m_ready_lock;            ///< 用于m_ready的锁
    Scheduler *   m_scheduler{nullptr};    ///< 所在调度器
    Task *        m_running_task{nullptr}; ///< 单前运行的任务
    condition_variable m_cv;              
};

#endif
