#ifndef __COMMON_H
#define __COMMON_H

#include <type_traits>
#include <functional>
#include <thread>
#include <chrono>
#include <sys/time.h>
#include <iostream>

//template< class T, class U >
//inline constexpr bool is_same_v = std::is_same<T, U>::value;

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;

struct ThreadHelper
{
    ThreadHelper(std::function<void()> f)
    {
        std::thread t(f);
        t.detach();
    }
};

static inline std::chrono::steady_clock::duration 
timeval2dur(struct timeval *tv)
{
    return std::chrono::seconds(tv->tv_sec) + 
        std::chrono::microseconds(tv->tv_usec);
}

#ifdef DEBUG
#define DEBUG_DETAIL(x) \
do{\
    std::cerr << "file " << __FILE__ << " function " << __func__ << \
    " line " << __LINE__ << " " << x << std::endl;\
}while(0)

#define DEBUGOUT(x) \
do{\
    std::cerr << x << std::endl;\
}while(0)

#else
#define DEBUGOUT(x) (void)0
#endif

#endif
