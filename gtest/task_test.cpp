#include <thread>
#include <iostream>
#include <stack>
#include <vector>
#include <gtest/gtest.h>
#include "../src/task.h"
#include "../src/taskallocator.h"
#include "../src/coroutine.h"

using namespace std;
using namespace co;

TEST(TaskStackAllocTest, MultiThreadAllocAndFree)
{
    constexpr size_t THREAD_CNT = 32;
    constexpr size_t COUNT = 10000;
    co_setstacksize(1UL << 10);
    vector<thread> vec;
    for(int j = 0; j < THREAD_CNT; j++)
    {
        vec.push_back(thread ([]{
            auto &tsalloc = TaskStackAllocator::getInstance();
            stack<char *> sta;
            for(int i = 0; i < COUNT; i++)
            {
                if(i & 1)
                {
                    tsalloc.free(sta.top());
                    sta.pop();
                }
                else 
                {
                    sta.push(static_cast<char *>(tsalloc.alloc()));
                }
            }
        }));
    }

    for(auto &t:vec)
        t.join();

    auto ret = TaskStackAllocator::getInstance().statistic();
    ASSERT_EQ(ret.first, ret.second);
}

TEST(TaskStackAllocTest, SingleThreadAllocAndFree)
{
    constexpr size_t COUNT = 32;
    auto &tsalloc = TaskStackAllocator::getInstance();
    co_setstacksize(1UL << 16);
    stack<char *>sta;
    for(int i = 0; i < COUNT; i++)
        sta.push(static_cast<char *>(tsalloc.alloc()));

    for(int i = 0; i < COUNT; i++)
    {
        tsalloc.free(sta.top());
        sta.pop();
    }
    auto ret = tsalloc.statistic();
    ASSERT_EQ(ret.first, ret.second);
}

TEST(TaskAllocTest, MultiThreadAllocAndFree)
{
    constexpr size_t COUNT = 1024;
    constexpr size_t THREAD_CNT = 16;
    co_setstacksize(1UL << 16);
    size_t stacksize = co_getstacksize();
    vector<thread> vec;
    for(int i = 0; i < THREAD_CNT; i++)
    {
        atomic<size_t> ids{0};
        vec.push_back(thread ([&ids, stacksize]{

            stack<Task *> sta;
            for(int i = 0; i < COUNT; i++)
            {
                if(i & 1)
                {
                    delete sta.top();
                    sta.pop();
                }
                else 
                {
                    sta.push(new Task([]{}, ids++, stacksize));
                }
            }
        }));
    }
    for(auto &t:vec)
        t.join();
    auto ret = TaskAllocator::getInstance().statistic();
    ASSERT_EQ(ret.first, ret.second);
}
