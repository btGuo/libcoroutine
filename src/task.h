#ifndef __TASK_H
#define __TASK_H

#include <functional>
#include <ucontext.h>

namespace co
{
/**
 * 任务状态
 */
enum class TaskStatus
{
    TaskRunning, TaskDead, TaskSuspend, TaskReady,
};

class Task
{
public:
    using TaskFn = std:: function<void()>;
    Task(TaskFn fn, std:: size_t id, std:: size_t stack_size);
    ~Task();
    static void taskMain(uint32_t low32, uint32_t high32);
    void yield(ucontext_t *pctx);
    void initContext(ucontext_t *pctx);
    void swapIn(ucontext_t *pctx);
    bool taskDead();
    void *operator new(std:: size_t size);
    void operator delete(void *ptr);

private:
    TaskFn     m_fn;
    ucontext_t m_ctx;
    TaskStatus m_status;
    char *     m_stack{nullptr};
    std:: size_t     m_stack_size;
    std:: size_t     m_id;
};

}

#endif
