#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "header.h"

using namespace std;

void func()
{
    cout << "coroutine by func" << endl;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        cout << "usage: " << argv[0] << " [threadcount]" << endl;
        return 0;
    }

    /**
     * co_getstacksize()查看默认栈大小
     */
    cout << "stack size " << co_getstacksize() << endl;

    /**
     * co_setstacksize()更改默认栈大小，注意只能在协程未创建前更改，栈大小会以系统
     * 页大小取整
     */
    co_setstacksize(1 << 16);

    /**
     * co_create()创建协程，此时协程只是被创建并没有开始运行
     */

    /**
     * 用lambda创建协程，lambda不能带有参数，返回值，类型是function<void()>
     */
    co_create([]{
        cout << "coroutine by lambda" << endl;
    });

    /**
     * 普通函数创建协程
     */
    co_create(func);

    /*
    //创建一个容量为0的通道
    co_chan<int> chan;

    //通道不可复制，必须传引用
    co_create([&chan]{
        cout << "consumer" << endl;
        int item;
        chan >> item;
        cout << "get item " << item << endl;
    });

    co_create([&chan]{
        cout << "productor" << endl;
        chan << 10;
        cout << "produce done" << endl;
    });
     */      
    /**
     * co_sched().start()开始调度协程，这将会阻塞当前线程，直到所有协程运行完成
     */
    co_sched().start(atoi(argv[1]));
}
