#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/poll.h>
#include <errno.h>

#include "processor.h"
#include "handlermap.h"
#include "fdcontext.h"
//#include "reactor.h"
#include "scheduler.h"
#include "common.h"

#define _GNU_SOURCE 1

using socket_t  = int(*)(int, int, int);
using accept_t  = int(*)(int, struct sockaddr *, socklen_t *);
using read_t    = ssize_t(*)(int, void *, size_t);
using write_t   = ssize_t(*)(int, const void *, size_t);
using connect_t = int(*)(int, const struct sockaddr *, socklen_t);
using fcntl_t   = int(*)(int, int, ...);
using getsockopt_t = int(*)(int, int, int, void *, socklen_t *);
using setsockopt_t = int(*)(int, int, int, const void *, socklen_t);
using close_t      = int(*)(int);

co::HandlerMap<co::FdContext *> g_contexts;
//static co::Reactor g_reactor;
static bool hook_enable{true};

static socket_t  socket_fn  = nullptr;
static accept_t  accept_fn  = nullptr;
static read_t    read_fn    = nullptr;
static write_t   write_fn   = nullptr;
static connect_t connect_fn = nullptr;
static fcntl_t   fcntl_fn   = nullptr;
static getsockopt_t getsockopt_fn = nullptr;
static setsockopt_t setsockopt_fn = nullptr;
static close_t close_fn = nullptr;

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
    handle_dlsym(fcntl_fn, "fcntl");
    handle_dlsym(getsockopt_fn, "getsockopt");
    handle_dlsym(setsockopt_fn, "setsockopt");
    handle_dlsym(close_fn, "close");

    //初始化调度器类
    co::Scheduler::getInstance();
}

void __attribute__((destructor)) co_hook_dtor()
{
}

#define getProcessor() co::Processor::getThisThreadProcessor()

static inline bool disable()
{
    return (!co::Scheduler::getInstance().running() || !hook_enable);
}

static inline int nonblock(int fd)
{
    int opts = fcntl_fn(fd, F_GETFL);
    if(opts < 0)
        return -1;
    opts = opts | O_NONBLOCK;
    if(fcntl_fn(fd, F_SETFL, opts) < 0)
        return -1;
    return 0;
}

/**
 * @tparam ret_t 返回值类型
 * @tparam func_t 函数类型
 * @tparam Args 可变参数
 *
 * @note 注意参数位置
 */
template <typename ret_t, typename func_t, typename ... Args> 
ret_t do_hook(func_t fn, co::EventType type, int fd, Args && ... args)
{
    auto *fdctx = g_contexts.get(fd);
    if(disable() || !fdctx || fdctx->isNonBlocking())
        return fn(fd, std::forward<Args>(args)...);
    
    auto *p = getProcessor();
    fdctx->add(p->getSuspendEntry(), type);
    //g_reactor.registerHandler(fdctx, type);
    p->taskBlock();
    //g_reactor.removeHandler(fdctx);
    
    return fn(fd, std::forward<Args>(args)...);
}

int close(int fd)
{
    DEBUGOUT("hook close");
    auto *fdctx = g_contexts.get(fd);
    if(disable() || !fdctx)
        return close_fn(fd);

    g_contexts.erase(fdctx->getHandler());
    free(fdctx);
    return close_fn(fd);
}

int socket(int domain, int type, int protocol)
{
    DEBUGOUT("hook socket");
    int ret = -1;
    if((ret = socket_fn(domain, type, protocol)) == -1)
        return -1;

    if(disable())
        return ret;
    // 设为nonblocking
    if(nonblock(ret) == -1)
        return -1;

    co::FdContext *fdctx = new co::FdContext(ret);
    g_contexts.insert(ret, fdctx);
    return ret;
}

int accept(int socket, struct sockaddr *address,
           socklen_t *address_len)
{
    DEBUGOUT("hook accept");
    int ret = do_hook<int>(accept_fn, co::EventType::EventRead,
                           socket, address, address_len);

    //设置为非阻塞
    if(ret == -1 || nonblock(ret) == -1)
        return ret;

    co::FdContext *fdctx = new co::FdContext(ret);
    g_contexts.insert(ret, fdctx);
    return ret;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    DEBUGOUT("hook read");
    return do_hook<ssize_t>(read_fn, co::EventType::EventRead,
                            fildes, buf, nbyte);
}

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    DEBUGOUT("hook write");
    return do_hook<ssize_t>(write_fn, co::EventType::EventWrite, 
                            fildes, buf, nbyte);
}

// 参考自libco
int connect(int sockfd, const struct sockaddr *addr, 
            socklen_t addrlen)
{
    DEBUGOUT("hook connect");
    int ret = connect_fn(sockfd, addr, addrlen);
    auto *fdctx = g_contexts.get(sockfd);

    //1. sys call
    if(disable() || !fdctx || fdctx->isNonBlocking())
    {
        return ret;
    }

    if(!(ret == -1 && errno == EINPROGRESS))
    {
        return ret;
    }

	//2.wait
	int pollret = 0;
	struct pollfd pf = {0};

	for(int i = 0; i < 3; i++) //25s * 3 = 75s
	{
		memset( &pf,0,sizeof(pf) );
		pf.fd = sockfd;
		pf.events = (POLLOUT | POLLERR | POLLHUP);

		pollret = poll(&pf, 1, 25000);

		if(pollret == 1)
		{
			break;
		}
	}
	if(pf.revents & POLLOUT) //connect succ
	{
		errno = 0;
		return 0;
	}

    int err = 0;
    socklen_t errlen = sizeof(err);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen);
    if(err)
        errno = err;
    else 
        errno = ETIMEDOUT;

    return ret;
}

//TODO 
int fcntl(int fildes, int cmd, ...)
{
    DEBUGOUT("hook fcntl");

    va_list args;
    va_start(args, cmd);
    int ret = 0;

    switch(cmd)
    {
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        {
            int third = va_arg(args, int);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
        case F_GETFD:
        {
            ret = fcntl_fn(fildes, cmd);
            break;
        }
        case F_SETFD:
        {
            int third = va_arg(args, int);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
        case F_GETFL:
        {
            ret = fcntl_fn(fildes, cmd);
            break;
        }
        case F_SETFL:
        {
            co::FdContext *fdctx = g_contexts.get(fildes);
            int third = va_arg(args, int);
            if(disable() || !fdctx)
                return fcntl_fn(fildes, cmd, third);

            int ret = fcntl_fn(fildes, cmd, third | O_NONBLOCK);
            if(fdctx && ret == 0)
                fdctx->addFlag(third);
            break;
        }
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        {
            struct flock *third = va_arg(args, struct flock *);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
        case F_OFD_SETLK:
        case F_OFD_SETLKW:
        case F_OFD_GETLK:
        {
            struct flock *third = va_arg(args, struct flock *);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
        case F_GETOWN:
        {
            ret = fcntl_fn(fildes, cmd);
            break;
        }
        case F_SETOWN:
        {
            int third = va_arg(args, int);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
        case F_GETOWN_EX:
        case F_SETOWN_EX:
        {
            struct f_owner_ex *third = va_arg(args, struct f_owner_ex *);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
        case F_GETSIG:
        {
            ret = fcntl_fn(fildes, cmd);
            break;
        }
        case F_SETSIG:
        {
            int third = va_arg(args, int);
            ret = fcntl_fn(fildes, cmd, third);
            break;
        }
    }
    va_end(args);

    return ret;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    int ret = setsockopt_fn(sockfd, level, optname, optval, optlen);
    auto *fdctx = g_contexts.get(sockfd);
    if(!fdctx)
        return ret;

    if(ret == 0 && level == SOL_SOCKET)
    {
        if(optname == SO_RCVTIMEO)
            fdctx->setRecvTimeOut(timeval2dur((struct timeval *)optval));
        else if(optname == SO_SNDTIMEO)
            fdctx->setSendTimeOut(timeval2dur((struct timeval *)optval));
    }
    return ret;
}
