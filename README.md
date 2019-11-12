一个简单的用户态线程（协程）库。目前只能运行在linux平台上。

###### 使用方法：

```
co_create(Taskfn fn, void *args);
```
创建新任务，只是简单的加入任务队列，并没有开始运行任务。

```
co_yield();
```
主动让出cpu;

```
co_sched.start(int min, int max);
```

开始调度任务，会阻塞调用方，直到所有的任务都做完，可以指定最小调度线程数和最大调度线程数。

###### 编译：

cd  src && make 会生成libcoroutine.a 静态库







