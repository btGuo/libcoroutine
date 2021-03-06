#ifndef __TASK_H
#define __TASK_H

#include <functional>
#include <ucontext.h>
#include <atomic>
//#include <sys/time.h>

namespace co
{
/**
 * 任务状态
 */
enum class TaskStatus
{
    TaskRunning, TaskDead, TaskSuspend, TaskReady, TaskBlock,
};

class Processor;

class Task
{
public:
    using TaskFn = std::function<void()>;
    Task(TaskFn fn, std::size_t id, std::size_t stack_size);
    ~Task();
    static void taskMain(uint32_t low32, uint32_t high32);
    std::size_t getId();
    //bool isExpire();
 //   void block(ucontext_t *pctx, struct timeval &tv);
    void yield(ucontext_t *pctx);
    void block(ucontext_t *pctx);
    void initContext(ucontext_t *pctx);
    void swapIn(ucontext_t *pctx);
    bool taskDead();
    bool taskBlock();
    void setProcessor(Processor *p);
    Processor *getProcessor();
    void *operator new(std::size_t size);
    void operator delete(void *ptr);
    bool isExpire(std::size_t expected);
    std::size_t getSuspendId();

private:
    Processor               *m_proc{nullptr};
    TaskFn                   m_fn;
    ucontext_t               m_ctx;
    TaskStatus               m_status;
    char                    *m_stack{nullptr};
    std::size_t              m_id;
    std::size_t              m_stack_size;
    std::atomic<std::size_t> m_suspendid{0};
};

}//namespace co

#endif
