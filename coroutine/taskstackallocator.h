#ifndef __TASK_STACK_ALLOCATOR_H
#define __TASK_STACK_ALLOCATOR_H

#include "singleton.h"
#include "freelist.h"

namespace co
{

/**
 * 分配任务栈，采用freelist进行回收，只能分配固定大小的栈，单例模式
 */
class TaskStackAllocator:public FreeList, public Singleton<TaskStackAllocator>
{
    friend class Singleton<TaskStackAllocator>;
public:

    void *alloc();
    void free(void *ptr);
    void setSize(std::size_t size);

private:
    std::size_t m_sys_pagesize{0}; ///< 系统页面大小

    void setProtect(void *addr);
    void *_alloc();
    TaskStackAllocator();
    ~TaskStackAllocator();

    TaskStackAllocator(const TaskStackAllocator &) = delete;
    TaskStackAllocator(TaskStackAllocator &&) = delete;
    TaskStackAllocator &operator = (const TaskStackAllocator &) = delete;
    TaskStackAllocator &operator = (TaskStackAllocator &&) = delete;
};

}
#endif
