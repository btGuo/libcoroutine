c++ stackful 协程实现。目前只能运行在linux平台上。

###### **使用方法**：

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

```
co_chan<T> chan;
```
创建类型为T的通道，通道默认大小为0。  

```
T obj;
chan << obj;
```
写通道，可能会导致阻塞。 

```
T obj;
chan >> obj;
```
读通道，可能会导致阻塞。

```
co_hook_enable();
```

允许hook系统调用，默认选项。

```
co_hook_disable();
```

禁止hook系统调用。

```
co_sleep(duration dur)
```

睡眠一段时间。

###### 编译：

cd  src && make 会生成libcoroutine.a 静态库  

###### 特点:  

使用M:N模型，即多个线程对应多个协程，采用工作窃取算法进行负载均衡，实现了一个无锁的工作窃取队列。关于协程栈，使用的是独立栈，默认条件下每次分配1M的栈空间。  

###### 注意事项:

由于协程采用的是M:N模型，需要共享数据时用通道实现，不要使用互斥锁同步原语。

###### 单元测试:

使用gtest进行单元测试。  

