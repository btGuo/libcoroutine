c++的协程实现。目前只能运行在linux平台上。

###### 使用方法：

```
co_create(Taskfn fn, size_t stacksize);
```
创建新任务，只是简单的加入任务队列，并没有开始运行任务。

```
co_yield();
```
主动让出cpu;

```
co_sched().start(int min, int max);
```

开始调度任务，会阻塞调用方，直到所有的任务都做完，可以指定最小调度线程数和最大调度线程数。
```
co_setstacksize(size_t stacksize);
```
设置默认协程栈大小，在协程未创建前调用，之后的设置将不再有效。
```
co_getstacksize();
```
获取默认协程栈大小。



###### 编译：

cd  src && make 会生成libcoroutine.a 静态库  

###### 特点:  

使用M:N模型，即多个线程对应多个协程，采用工作窃取算法进行负载均衡，实现了一个无锁的工作窃取队列。关于协程栈，使用的是独立栈，默认条件下每次使用mmap分配1M的栈空间。  

###### 单元测试:
使用gtest进行单元测试。  









