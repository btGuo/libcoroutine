#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include "workstealingqueue.h"

using namespace std;

#define DELAY_MAX 5453 
#define DELAY_MIN 244 

#define DELAY()\
do{ int __cnt = intRand(DELAY_MIN, DELAY_MAX);\
    for(int i = 0; i < __cnt; i++);\
}while(0)

#define COUNT 1000000
#define TASK_COUNT (1 << 20)
WorkStealingQueue<int> que;
mutex mtx;
int total = 0;

int intRand(const int & min, const int & max) 
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}

void owner()
{
    int count = 0;
    int result = 0;
    for(int i = 0; i < COUNT; i++)
    {
        if(que.pop(result))
            ++count;
        DELAY();
    }
    lock_guard<mutex> lock(mtx);
    //cout << "pop count " << count << endl;
    total += count;
}

void other()
{
    int count = 0;
    int result = 0;
    for(int i = 0; i < COUNT; i++)
    {
        if(que.steal(result))
            ++count;
        DELAY();
    }

    lock_guard<mutex> lock(mtx);
    //cout << "steal count " << count << endl;
    total += count;
}

#define BATCH_COUNT 10000

int main()
{
    for(int j = 1; j < BATCH_COUNT; j++)
    {
        que.clear();
        for(int i = 0; i < TASK_COUNT; i++)
            que.push(i);

        thread t(owner);

        total = 0;
        thread t1(other);
        thread t2(other);
        thread t3(other);
        thread t4(other);

        t.join();
        t1.join();
        t2.join();
        t3.join();
        t4.join();

        if(total > TASK_COUNT)
        {
            cout << "error" << endl;
            return 0;
        }
        cout << "test " << j << " done" << endl;
    }
}
