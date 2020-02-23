# coroutine

一个c++ stackful 协程库。目前只能运行在linux平台上，还有很多不完善的地方。

### 构建安装

获取源代码

```
git clone https://github.com/btGuo/libcoroutine.git
```

安装

```
cd coroutine
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make 
sudo make install
```

### 使用文档

包含头文件

```
#include <coroutine/coroutine.h>
```

编译链接选项

```
-lcoroutine -lpthread -ldl
```

创建新任务，只是简单的加入任务队列，并没有开始运行任务。

```
co_create(Taskfn fn, size_t stacksize);
```
主动让出cpu;

```
co_yield();
```
开始调度任务，会阻塞调用方，直到所有的任务都做完，可以指定最小调度线程数和最大调度线程数。

```
co_sched().start(int min, int max);
```
设置默认协程栈大小，在协程未创建前调用，之后的设置将不再有效。

```
co_setstacksize(size_t stacksize);
```
 获取默认协程栈大小。

```
co_getstacksize();
```
创建类型为T的通道，通道默认大小为0。

```
co_chan<T> chan;
```
写通道，可能会导致阻塞。  

```
T obj;
chan << obj;
```
读通道，可能会导致阻塞。 

```
T obj;
chan >> obj;
```
允许hook系统调用，默认选项。

```
co_hook_enable();
```

禁止hook系统调用。

```
co_hook_disable();
```

睡眠一段时间。

```
co_sleep(duration dur)
```

详细例子位于tutorial文件夹下面

example.cpp 使用教程，tcpserver.cpp, tcpclient.cpp 一个tcp echo客户端服务器例子。

### 目前hook的系统调用

```
socket
accept
read
write
connect
fcntl
getsockopt
setsockopt
close
```

### 特点  

使用M:N模型，即多个线程对应多个协程，采用工作窃取算法进行负载均衡，实现了一个无锁的工作窃取队列。关于协程栈，使用的是独立栈，默认条件下每次分配1M的栈空间。  关于协程栈的一些见解在这里 [协程栈](https://btguo.github.io/jekyll/update/2019/11/10/the-stack-of-coroutine.html)

#### 注意事项

由于协程采用的是M:N模型，需要共享数据时用通道实现，不要使用互斥锁同步原语。

