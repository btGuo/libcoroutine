#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include <list>
#include <vector>
#include "../coroutine/processor.h"
#include "../coroutine/singleton.h"

namespace co
{

class Timer;

/**
 * 调度器类，单例模式
 */
class Scheduler:public Singleton<Scheduler> 
{
    friend class Singleton<Scheduler>;
public:
    /**
     * 启动调度器
     * @param min: 最小调度线程数，为0时，设置为cpu核心数，
     * @param max: 最大调度线程数，为0时，设置为min，
     * @note 如果max大于min，有必要时会自动扩充调度线程数.
     */
    void start(int min = 1, int max = 0);
    /**
     * 获取m_processors队列大小
     */
    std::size_t getProcessorSize();
    /**
     * 获取m_processors[pos]元素
     */
    Processor *getProcessor(std::size_t pos);
    /**
     * 打印统计信息
     */
    void showStatistics();
    bool running();
    Timer *getTimer();

    void setStackSize(std::size_t stack_size);
    std::size_t getStackSize();
    void create(Task::TaskFn fn, std::size_t stack_size);
    void yield();
    void sleep(std::chrono::steady_clock::duration dur);

private:

    std::vector<Processor *> m_processors;          ///< 所有processor队列
    Processor               *m_self{nullptr};       ///< 调度器所在的processor
    int                      m_min_threads{0};      ///< 最小调度线程数
    int                      m_max_threads{0};      ///< 最大调度线程数
    bool                     m_start{false};        ///< 启动标志
    std::size_t              m_ids{0};              ///< 用于给processor分配id
    Timer                   *m_timer{nullptr};

    Scheduler();
    ~Scheduler();
    Scheduler(Scheduler const&) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler& operator=(Scheduler const&) = delete;
    Scheduler& operator=(Scheduler &&) = delete;
};

}

#endif
