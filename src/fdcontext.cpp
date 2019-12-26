#include "fdcontext.h"
#include "demultiplexer.h"
#include "common.h"
#include <fcntl.h>

extern co::HandlerMap<co::FdContext *> g_contexts;

namespace co
{

#ifdef __linux__
EventDemultiplexer *FdContext::m_demultiplexer = new EpollDemultiplexer();
#else 
EventDemultiplexer *FdContext::m_demultiplexer = nullptr;
#endif

FdContext::FdContext(int fd)
{
    m_fd = fd;
}

int FdContext::getHandler()
{
    return m_fd;
}

void FdContext::handlerWrite()
{
    std::unique_lock<std::mutex> lg(m_mtx);
    auto entry = m_wque.front();
    m_wque.pop_front();
    lg.unlock();
    entry.t->getProcessor()->taskWakeup(entry);
    m_demultiplexer->removeEvent(m_fd);
}

void FdContext::handlerRead()
{
    std::unique_lock<std::mutex> lg(m_mtx);
    auto entry = m_rque.front();
    m_rque.pop_front();
    lg.unlock();
    entry.t->getProcessor()->taskWakeup(entry);
    m_demultiplexer->removeEvent(m_fd);
}

void FdContext::handlerError()
{
}

void FdContext::setRecvTimeOut(duration_t dur)
{
    m_rtime = dur;
}

void FdContext::setSendTimeOut(duration_t dur)
{
    m_wtime = dur;
}

FdContext::duration_t FdContext::getRecvTimeOut()
{
    return m_rtime;
}

FdContext::duration_t FdContext::getSendTimeOut()
{
    return m_wtime;
}

void FdContext::add(Processor::SuspendEntry entry, EventType type)
{
    static ThreadHelper thh([this]{ 
        this->m_demultiplexer->waitEvent(g_contexts); 
    });

    std::lock_guard<std::mutex> lg(m_mtx);
    if(type == EventType::EventRead)
        m_rque.push_back(entry);
    else if(type == EventType::EventWrite)
        m_wque.push_back(entry);
    else 
        return;

    m_demultiplexer->addEvent(m_fd, type);
}

void FdContext::addFlag(int flag)
{
    m_flag |= flag;
}

bool FdContext::isNonBlocking()
{
    return m_flag & O_NONBLOCK;
}

}//namespace co
