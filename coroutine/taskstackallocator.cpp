#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include "freelist.h"
#include "taskstackallocator.h"

namespace co
{

void *TaskStackAllocator::alloc()
{
    void *ret = freelistAlloc();
    if(ret) // 在freelist中ret指向最后一个页面
        return static_cast<char *>(ret) - m_size + m_sys_pagesize;
    return _alloc();
}

void TaskStackAllocator::free(void *ptr)
{
    // 指向最后一个页面处
    freelistFree(static_cast<char *>(ptr) + m_size - m_sys_pagesize);
}

void TaskStackAllocator::setSize(std::size_t size)
{
    static bool mark = false;
    if(!mark)
    {
        //对page_size上取整
        size = (size + m_sys_pagesize - 1) & ~(m_sys_pagesize - 1);
        mark = true;
    }
}

void TaskStackAllocator::setProtect(void *addr)
{
    assert(mprotect(addr, m_sys_pagesize, PROT_NONE) != -1);
}

void *TaskStackAllocator::_alloc()
{
    //多分配一个页面
    void *ret = mmap(NULL, m_size + m_sys_pagesize, PROT_READ|PROT_WRITE,
                MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    assert(ret != (void*)-1);
    //第一个页面设置禁止读写
    setProtect(ret);
    //跳过保护页面
    return static_cast<char *>(ret) + m_sys_pagesize;
}

TaskStackAllocator::TaskStackAllocator()
{
    int pagesize = sysconf(_SC_PAGE_SIZE);
    if(pagesize == -1)
        pagesize = 4096;

    m_sys_pagesize = pagesize;
    m_size = 1UL << 20;
}

TaskStackAllocator::~TaskStackAllocator()
{
}

}
