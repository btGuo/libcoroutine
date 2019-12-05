#include "task.h"
#include "taskallocator.h"
#include "taskstackallocator.h"
#include <stdlib.h>

namespace co
{

Task::Task(TaskFn fn, size_t id, size_t stack_size):
    m_fn(fn), m_id(id), m_stack_size(stack_size)
{
    if(m_stack_size == TaskStackAllocator::getInstance().getSize())
        m_stack = static_cast<char *>(TaskStackAllocator::getInstance().alloc());
    else 
        m_stack = static_cast<char *>(malloc(m_stack_size));

    m_status = TaskStatus:: TaskReady;
}

Task::~Task()
{
    if(m_stack_size == TaskStackAllocator::getInstance().getSize())
        TaskStackAllocator::getInstance().free(m_stack);
    else 
        free(m_stack);
}

void Task:: taskMain(uint32_t low32, uint32_t high32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
    Task *task = (Task *)ptr;
    task->m_fn();
    task->m_status = TaskStatus:: TaskDead;
}

void Task:: yield(ucontext_t *pctx)
{
    m_status = TaskStatus:: TaskSuspend;
    swapcontext(&m_ctx, pctx);
}

void Task:: initContext(ucontext_t *pctx)
{
    getcontext(&m_ctx);
    this->m_ctx.uc_stack.ss_sp = m_stack;
    this->m_ctx.uc_stack.ss_size = m_stack_size; 
    this->m_ctx.uc_link = pctx; 
    uintptr_t ptr = (uintptr_t)this;
    makecontext(&this->m_ctx, (void (*)(void))taskMain, 2, 
            (uint32_t)ptr, (uint32_t)(ptr >> 32));
}

void Task:: swapIn(ucontext_t *pctx)
{
    if(m_status == TaskStatus:: TaskReady)
        initContext(pctx);
    swapcontext(pctx, &m_ctx);
}

bool Task:: taskDead()
{
    return m_status == TaskStatus:: TaskDead;
}

void *Task:: operator new(size_t size)
{
    return TaskAllocator::getInstance().alloc();       
}

void Task:: operator delete(void *ptr)
{
    return TaskAllocator::getInstance().free(ptr);
}

}
