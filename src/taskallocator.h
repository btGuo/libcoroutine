#ifndef __TASK_ALLOCATOR_H
#define __TASK_ALLOCATOR_H

#include "singleton.h"
#include "freelist.h"

namespace co
{
/**
 * 任务内存分配器，采用freelist回收，只用于分配固定大小内存，单例模式
 */
class TaskAllocator:public FreeList, public Singleton<TaskAllocator>
{
    friend class Singleton<TaskAllocator>;
public:
    void *alloc();
    void free(void *ptr);
private:
    void *_alloc();

    TaskAllocator();
    TaskAllocator(const TaskAllocator &) = delete;
    TaskAllocator(TaskAllocator &&) = delete;
    TaskAllocator &operator = (const TaskAllocator &) = delete;
    TaskAllocator &operator = (TaskAllocator &&) = delete;
};

}
#endif
