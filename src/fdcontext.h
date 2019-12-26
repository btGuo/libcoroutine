#ifndef __FD_CONTEXT_H
#define __FD_CONTEXT_H

#include <list>
#include <chrono>
#include <mutex>
#include "processor.h"
//#include "reactor.h"

namespace co
{

enum class EventType
{
    EventRead, EventWrite, EventError
};

class EventDemultiplexer;

class FdContext//:public EventHandler
{
public:
    using duration_t = std::chrono::steady_clock::duration;
    FdContext(int fd);
    int getHandler();
    void handlerWrite();
    void handlerRead();
    void handlerError();
    void add(Processor::SuspendEntry entry, EventType type);
    void addFlag(int flag);
    void setRecvTimeOut(duration_t dur);
    void setSendTimeOut(duration_t dur);
    duration_t getRecvTimeOut();
    duration_t getSendTimeOut();
    bool isNonBlocking();
private:
    int                                m_fd{-1};
    duration_t                         m_rtime;
    duration_t                         m_wtime;
    int                                m_flag{0};
    std::mutex                         m_mtx;
    std::list<Processor::SuspendEntry> m_rque;
    std::list<Processor::SuspendEntry> m_wque;
    static EventDemultiplexer         *m_demultiplexer;
};

}//namespace co

#endif
