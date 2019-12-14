#ifndef __FREE_LIST_H
#define __FREE_LIST_H

#include <atomic>
#include <utility>

namespace co
{

class FreeList
{
public:
    std::size_t getSize();
    virtual void setSize(std::size_t size);
    std::pair<std::size_t, std::size_t> statistic();
protected:
    void *freelistAlloc();
    void freelistFree(void *);
    std::size_t m_size{0};
private:
    struct ListElem
    {
        ListElem *next;
    };
    std::atomic<std::size_t> m_alloc{0};
    std::atomic<std::size_t> m_free{0};
    std::atomic<ListElem *>  m_head{nullptr};
};

}
#endif
