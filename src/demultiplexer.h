#ifndef __DEMULTIPLEXER_H
#define __DEMULTIPLEXER_H

#include "threadsafequeue.h"
#include "singleton.h"
#include "task.h"
#include <mutex>
#include <atomic>

namespace co
{
class EpollDemultiplexer:public Singleton<EpollDemultiplexer>
{
public:
    SingletonDeclare(EpollDemultiplexer);
    enum class EventType 
    {
        EventRead, EventWrite
    };
    void addEvent(int fd, EventType type, Task *t);
    void removeEvent(int fd);
private:

    void run();
    void init();
    std::mutex                m_mtx;
    int                       m_epfd{-1};
    std::size_t               m_max_events{1024};
    bool                      m_start{false};
};

}//namespace co

#endif
