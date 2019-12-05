#include <stdlib.h>
#include "taskallocator.h"
#include "task.h"

namespace co
{

void *TaskAllocator:: alloc()
{
    void *ret = freelistAlloc();
    if(ret)
        return ret;
    return _alloc();
}

void TaskAllocator:: free(void *ptr)
{
    freelistFree(ptr);
}

void *TaskAllocator:: _alloc()
{
    return malloc(m_size);
}

TaskAllocator:: TaskAllocator()
{
    m_size = sizeof(Task);
}
}
