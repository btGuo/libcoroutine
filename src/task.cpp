#include "taskallocator.h"
#include "taskstackallocator.h"
#include "processor.h"
#include "task.h"
#include <stdlib.h>
#include <iostream>

namespace co
{

Task::Task(TaskFn fn, size_t id, size_t stack_size):
    m_fn(fn), m_id(id), m_stack_size(stack_size)
{
    if(m_stack_size == TaskStackAllocator::getInstance().getSize())
        m_stack = static_cast<char *>(TaskStackAllocator::getInstance().alloc());
    else 
        m_stack = static_cast<char *>(malloc(m_stack_size));

    m_status = TaskStatus::TaskReady;
}

Task::~Task()
{
    //std::cout << "dtor" << std::endl;
    if(m_stack_size == TaskStackAllocator::getInstance().getSize())
        TaskStackAllocator::getInstance().free(m_stack);
    else 
        free(m_stack);
}

void Task::taskMain(uint32_t low32, uint32_t high32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
    Task *task = (Task *)ptr;
    task->m_fn();
    task->m_status = TaskStatus::TaskDead;
}

void Task::block(ucontext_t *pctx)
{
    m_status = TaskStatus::TaskBlock;
    swapcontext(&m_ctx, pctx);
}

void Task::yield(ucontext_t *pctx)
{
    m_status = TaskStatus::TaskSuspend;
    swapcontext(&m_ctx, pctx);
}

void Task::initContext(ucontext_t *pctx)
{
    getcontext(&m_ctx);
    this->m_ctx.uc_stack.ss_sp = m_stack;
    this->m_ctx.uc_stack.ss_size = m_stack_size; 
    this->m_ctx.uc_link = pctx; 
    uintptr_t ptr = (uintptr_t)this;
    makecontext(&this->m_ctx, (void (*)(void))taskMain, 2, 
            (uint32_t)ptr, (uint32_t)(ptr >> 32));
}

void Task::swapIn(ucontext_t *pctx)
{
    if(m_status == TaskStatus::TaskReady)
        initContext(pctx);
    swapcontext(pctx, &m_ctx);
}

bool Task::taskDead()
{
    return m_status == TaskStatus::TaskDead;
}

bool Task::taskBlock()
{
    return m_status == TaskStatus::TaskBlock;
}

void Task::setProcessor(Processor *p)
{
    m_proc = p;
}

Processor *Task::getProcessor()
{
    return m_proc;
}

void *Task::operator new(size_t size)
{
    return TaskAllocator::getInstance().alloc();       
}

void Task::operator delete(void *ptr)
{
    //std::cout << "operator delete" << std::endl;
    return TaskAllocator::getInstance().free(ptr);
}

size_t Task::getId()
{
    return m_id;
}

}
