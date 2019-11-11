#ifndef __COROUTINE_H
#define __COROUTINE_H

#include "scheduler.h"

#define co_create(fn, args) (Scheduler::getInstance().addTask((TaskFn)fn, args))
#define co_yield() (Processor::getProcessor()->taskYield())

#define co_sched Scheduler::getInstance()

#endif
