#ifndef __COMMON_H
#define __COMMON_H

#include <type_traits>

//template< class T, class U >
//inline constexpr bool is_same_v = std::is_same<T, U>::value;

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;

#define __DEBUG 1

#ifdef __DEBUG
#define DEBUG_DETAIL(x) \
do{\
    std::cerr << "file " << __FILE__ << " function " << __func__ << \
    " line " << __LINE__ << " " << x << std::endl;\
}while(0)

#define DEBUG(x) \
do{\
    std::cerr << x << std::endl;\
}while(0)

#else
#define DEBUG(x)
#endif

#endif
