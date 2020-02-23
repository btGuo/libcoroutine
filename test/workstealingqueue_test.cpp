#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <gtest/gtest.h>
#include "../src/workstealingqueue.h"

using namespace std;
using namespace co;

constexpr int DELAY_MAX = 553;
constexpr int DELAY_MIN = 44; 
constexpr int BATCH_COUNT = 100;

int intRand(const int & min, const int & max) 
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}

void inline delay()
{
    int cnt = intRand(DELAY_MIN, DELAY_MAX);
    for(int i = 0; i < cnt; i++);
}

class WorkStealingQueueTest:public::testing::TestWithParam<int>{};

TEST_P(WorkStealingQueueTest, MultiThreadStealPopAndSteal)
{
    constexpr int COUNT = 100000;
    constexpr int TASK_COUNT = (1 << 16);
    constexpr int STEAL_COUNT = 4;

    WorkStealingQueue<int> que;
    mutex mtx;
    int total = 0;
    int taskcount = TASK_COUNT;

    for(int i = 0; i < TASK_COUNT; i++)
        que.push(i);

    thread productor([&que, &mtx, &total, &taskcount]{
        int count = 0;
        int result = 0;
        for(int i = 0; i < COUNT; i++)
        {
            if(i % 3)
            {
                que.push(i);
                taskcount++;
            }
            else if(que.pop(result))
            {
                ++count;
            }
            //delay();
        }
        lock_guard<mutex> lock(mtx);
        total += count;
    });

    vector<thread> others;
    
    for(int i = 0; i < STEAL_COUNT; i++)
    {
        others.push_back(thread ([&que, &mtx, &total]{

            int count = 0;
            int result = 0;
            for(int i = 0; i < COUNT; i++)
            {
                if(que.steal(result))
                    ++count;
                //delay();
            }
            lock_guard<mutex> lock(mtx);
            total += count;
        }));
    }

    productor.join();
    for(auto &t: others)
        t.join();

    ASSERT_LE(total, taskcount);
}

INSTANTIATE_TEST_CASE_P(Instantiation, WorkStealingQueueTest, ::testing::Range(1, BATCH_COUNT));
