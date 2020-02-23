#include "demultiplexer.h"
#include "task.h"
#include "processor.h"
#include "common.h"
#include <thread>
#include <sys/epoll.h>
#include <assert.h>
#include <iostream>
#include <thread>

#ifdef __linux__

namespace co
{

EpollDemultiplexer::EpollDemultiplexer()
{
    m_epfd = epoll_create(1);
    assert(m_epfd != -1);
}

EpollDemultiplexer::~EpollDemultiplexer()
{
    close(m_epfd);
}

void EpollDemultiplexer::removeEvent(int fd)
{
    std::lock_guard<std::mutex> lg(m_mtx);
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
}

void EpollDemultiplexer::addEvent(int fd, EventType type)
{
    DEBUGOUT("Epoll addEvent");
    epoll_event epev;
    epev.events = 0;
    if(type == EventType::EventRead)
        epev.events |= EPOLLIN;
    else if(type == EventType::EventWrite)
        epev.events |= EPOLLOUT;
    else 
        return;

    epev.data.fd = fd;
    // 是否多线程安全?
    std::lock_guard<std::mutex> lg(m_mtx);
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &epev);
}

void EpollDemultiplexer::waitEvent(HandlerMap<FdContext *> &handlers)
{
    for(;;)
    {
        epoll_event events[m_max_events];
        int epfd = epoll_wait(m_epfd, events, m_max_events, -1);
        assert(epfd != -1);
        for(int i = 0; i < epfd; i++)
        {
            DEBUGOUT("epoll wakeup");
            if(events[i].events & EPOLLIN ||
                    events[i].events & EPOLLRDNORM)
            {
                handlers.get(events[i].data.fd)->handlerRead();
            }
            else if(events[i].events & EPOLLOUT ||
                    events[i].events & EPOLLWRNORM)
            {
                handlers.get(events[i].data.fd)->handlerWrite();
            }
        }
    }
}

}// namespace co

#else

#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>

namespace co
{

// ======================================================================

void PollDemultiplexer::addEvent(int fd, EventType type, Task *t)
{
    EventEntry en = {fd, type, t};
    write(fd[1], &en, sizeof(en));
}

void PollDemultiplexer::init()
{
    assert(pipe(m_pipefd) != -1);
    //设置为非阻塞
    assert(fcntl(fd[0], F_SETFL, O_NONBLOCK) != -1);
    assert(fcntl(fd[1], F_SETFL, O_NONBLOCK) != -1);

    m_pollfd[m_pollfd].fd = m_pipefd[0];
    m_pollfd[m_pollfd].events |= POLLOUT;
    m_pollfd[m_pollfd].revents = 0;
}

void PollDemultiplexer::run()
{
    bool new_event;
    for(;;)
    {
        new_event = false;
        int nfds = poll(m_pollfd, m_npollfd, -1);

        if(m_pollfd[0].revents)
            new_event = true;
        for(int i = 1; i < m_npollfd; i++)
        {
            while(i < m_npollfd && m_pollfd[i].revents)
            {
                // 后面的挪到前面
                Task *t = m_tasks[m_npollfd];
                t->getProcessor()->taskWakeup(t);
                m_npollfd--;
                m_pollfd[i] = m_pollfd[m_npollfd];
                m_tasks[i]  = m_tasks[m_npollfd];
            }
        }

        if(new_event)
        {
            EventEntry en;
            int nread;
            while((nread = read(fd[0], &en, sizeof(en))) > 0)
            {
                assert(nread == sizeof(en));

                if(en.type == EventType::EventRead)
                    m_pollfd[m_npollfd].events |= POLLIN;
                else if(type == EventType::EventWrite)
                    m_pollfd[m_npollfd].events |= POLLOUT;
                else 
                    continue;
    
                m_pollfd[m_npollfd].fd = fd;
                m_pollfd[m_npollfd].revents = 0;
                m_tasks[m_npollfd] = t;
                m_npollfd++;
            }
        }
    }

}

}//namespace co

#endif
