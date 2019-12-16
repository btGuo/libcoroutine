#include "demultiplexer.h"
#include "task.h"
#include "processor.h"
#include "common.h"
#include <thread>
#include <sys/epoll.h>
#include <assert.h>
#include <iostream>
#include <thread>

namespace co
{
void EpollDemultiplexer::addEvent(int fd, EventType type, Task *t)
{
    std::lock_guard<std::mutex> lg(m_mtx);
    if(!m_start)
        init();

    epoll_event epev;
    if(type == EventType::EventRead)
        epev.events |= EPOLLIN;
    else if(type == EventType::EventWrite)
        epev.events |= EPOLLOUT;
    else 
        return;
    epev.data.ptr = t;
    // 是否多线程安全?
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &epev);
    DEBUG("EpollDemultiplexer addEvent done");
}

void EpollDemultiplexer::init()
{
    m_start = true;
    m_epfd = epoll_create(1);
    assert(m_epfd != -1);
    std::thread t([this]{
       this->run();
    });
    t.detach();
}

void EpollDemultiplexer::run()
{
    DEBUG("EpollDemultiplexer start run");
    for(;;)
    {
        epoll_event events[m_max_events];
        int epfd = epoll_wait(m_epfd, events, m_max_events, -1);
        assert(epfd != -1);
        for(int i = 0; i < epfd; i++)
        {
            if(events[i].events & EPOLLIN ||
                    events[i].events & EPOLLOUT)
            {
                Task *t = static_cast<Task *>(events[i].data.ptr);
                t->getProcessor()->taskWakeup(t);
            }
        }
    }
}

void EpollDemultiplexer::removeEvent(int fd)
{
    std::lock_guard<std::mutex> lg(m_mtx);
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
}

}//namespace co
