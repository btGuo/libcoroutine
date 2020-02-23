#include "freelist.h"

namespace co
{

std::size_t FreeList::getSize()
{
    return m_size;
}

void FreeList::setSize(std::size_t size)
{ 
    static bool mark = false;
    if(!mark)
    {
        m_size = size; 
        mark = true;
    }
}

std::pair<std::size_t, std::size_t> FreeList::statistic()
{
    return {m_alloc.load(), m_free.load()};
}

void *FreeList::freelistAlloc()
{
    ++m_alloc;
    ListElem *cur = m_head.load();
    while(cur && !m_head.compare_exchange_weak(cur, cur->next));

    return cur;
}

void FreeList::freelistFree(void *ptr)
{
    ++m_free;
    ListElem *elem = static_cast<ListElem *>(ptr);
    elem->next = m_head.load();
    while(!m_head.compare_exchange_weak(elem->next, elem));
}

}
