#ifndef __DEMULTIPLEXER_H
#define __DEMULTIPLEXER_H

#include "threadsafequeue.h"
#include "task.h"
#include "fdcontext.h"
#include "handlermap.h"
//#include "eventhandler.h"

#include <mutex>
#include <atomic>
#include <list>

namespace co
{

class EventDemultiplexer
{
public:
    virtual ~EventDemultiplexer(){}
    virtual void waitEvent(HandlerMap<FdContext *> &handlers) = 0;
    virtual void addEvent(int fd, EventType type) = 0;
    virtual void removeEvent(int fd) = 0;
private:
};

}//namespace co

#ifdef __linux__

namespace co
{

class EpollDemultiplexer:public EventDemultiplexer
{
public:
    EpollDemultiplexer();
    ~EpollDemultiplexer();
    void waitEvent(HandlerMap<FdContext *> &handlers);
    void addEvent(int fd, EventType type);
    void removeEvent(int fd);
    
private:
    std::mutex  m_mtx;
    int         m_epfd{-1};
    std::size_t m_max_events{1024};
    bool        m_start{false};
};

}// namespace co

#else

namespace co
{

class PollDemultiplexer
{
public:
    void addEvent(int fd, EventType type, Task *t);
    struct EventEntry
    {
        int fd;
        EventType type;
        Task *t;
    };
private:
    void run();
    void init();
    std::mutex            m_mtx;
    constexpr std::size_t m_maxfd{1024};
    struct pollfd         m_pollfd[m_maxfd];
    Task                 *m_tasks[m_maxfd];
    std::size_t           m_npollfd{0};
    int[2]                m_pipefd;
    std::list<EventEntry> m_events;
};

}//namespace co

#endif

#endif
