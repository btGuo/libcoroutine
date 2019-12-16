#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include "demultiplexer.h"
#include "processor.h"
#include "scheduler.h"
#include "common.h"

#define _GNU_SOURCE 1

using socket_t  = int(*)(int, int, int);
using accept_t  = int(*)(int, struct sockaddr *, socklen_t *);
using read_t    = ssize_t(*)(int, void *, size_t);
using write_t   = ssize_t(*)(int, void *, size_t);
using connect_t = int(*)(int, const struct sockaddr *, socklen_t);

static socket_t  socket_fn   = nullptr;
static accept_t  accept_fn   = nullptr;
static read_t    read_fn     = nullptr;
static write_t   write_fn     = nullptr;
static connect_t connect_fn = nullptr;

#define handle_dlsym(fn, name)\
do{\
    dlerror();\
    *(void **)(&fn) = dlsym(RTLD_NEXT, name);\
    if(dlerror() != NULL){\
        perror("dlsym");\
        exit(EXIT_FAILURE);\
    }\
}while(0)


void __attribute__((constructor)) co_hook_ctor()
{
    handle_dlsym(socket_fn, "socket");
    handle_dlsym(accept_fn, "accept");
    handle_dlsym(read_fn, "read");
    handle_dlsym(write_fn, "write");
    handle_dlsym(connect_fn, "connect");

    //初始化调度器类
    co::Scheduler::getInstance();
}

void __attribute__((destructor)) co_hook_dtor()
{
}

int socket(int domain, int type, int protocol)
{
    DEBUG("hook socket");
    int ret = -1;
    if((ret = socket_fn(domain, type, protocol)) == -1)
        return -1;

    if(!co::Scheduler::getInstance().running())
        return ret;

    // 设为nonblocking
    int opts = fcntl(ret, F_GETFL);
    if(opts < 0)
        return -1;
    opts = opts | O_NONBLOCK;
    if(fcntl(ret, F_SETFL, opts) < 0)
        return -1;
    return ret;
}

int accept(int socket, struct sockaddr *address,
           socklen_t *address_len)
{
    DEBUG("hook accept");
    if(!co::Scheduler::getInstance().running())
        return accept_fn(socket, address, address_len);
        
    co::Processor *p = co::Processor::getThisThreadProcessor();
    co::EpollDemultiplexer::getInstance().addEvent(socket, 
        co::EpollDemultiplexer::EventType::EventRead, p->getRunningTask());
    p->taskBlock();
    co::EpollDemultiplexer::getInstance().removeEvent(socket);
    return accept_fn(socket, address, address_len);
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    DEBUG("hook read");
    if(!co::Scheduler::getInstance().running())
        return read_fn(fildes, buf, nbyte);

    co::Processor *p = co::Processor::getThisThreadProcessor();
    co::EpollDemultiplexer::getInstance().addEvent(fildes, 
        co::EpollDemultiplexer::EventType::EventRead, p->getRunningTask());
    p->taskBlock();
    co::EpollDemultiplexer::getInstance().removeEvent(fildes);
    return read_fn(fildes, buf, nbyte);
}

ssize_t write(int fildes, void *buf, size_t nbyte)
{
    DEBUG("hook write");
    if(!co::Scheduler::getInstance().running())
        return write_fn(fildes, buf, nbyte);

    co::Processor *p = co::Processor::getThisThreadProcessor();
    co::EpollDemultiplexer::getInstance().addEvent(fildes, 
        co::EpollDemultiplexer::EventType::EventWrite, p->getRunningTask());
    p->taskBlock();
    co::EpollDemultiplexer::getInstance().removeEvent(fildes);
    return write_fn(fildes, buf, nbyte);
}

int connect(int sockfd, const struct sockaddr *addr, 
            socklen_t addrlen)
{
    DEBUG("hook connect");
    if(!co::Scheduler::getInstance().running())
        return connect_fn(sockfd, addr, addrlen);

    co::Processor *p = co::Processor::getThisThreadProcessor();
    co::EpollDemultiplexer::getInstance().addEvent(sockfd, 
        co::EpollDemultiplexer::EventType::EventWrite, p->getRunningTask());
    p->taskBlock();
    co::EpollDemultiplexer::getInstance().removeEvent(sockfd);
    return connect_fn(sockfd, addr, addrlen);
}
