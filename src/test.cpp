#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include "coroutine.h"

using namespace std;

void j1()
{
    cout << "j1 start\n";
    co_yield();
    cout << "j1 done\n";
}

void j2()
{
    cout << "j2 start\n";
    co_yield();
    cout << "j2 done\n";
}

void j3()
{
    for(int i = 0; i < 10000000; i++);
    cout << "j3\n";
}

int main(int argc, char **argv)
{
    if(argc < 2)
        return 0;

    for(int i = 0; i < 100; i++)
        co_create(j3, nullptr);

    co_sched.start(atoi(argv[1]));
}

