#ifndef __TASK_H
#define __TASK_H

#include <ucontext.h>
#include <unistd.h>

using TaskFn = void (*)(void *);

enum class TaskStatus
{
    TaskRunning, TaskDead, TaskSuspend, TaskReady,
};

struct Task
{
    TaskFn     fn;
    void *     args;
    ucontext_t ctx;
    TaskStatus status;
    char *     data{nullptr};
    size_t     cap{0};
    size_t     id;
};


#endif
