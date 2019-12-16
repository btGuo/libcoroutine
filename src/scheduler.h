#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include <list>
#include <vector>
#include "processor.h"
#include "singleton.h"

namespace co
{

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
    /**
     * 获取调度器实体
     */
    bool running();
private:

    std::vector<Processor *> m_processors;          ///< 所有processor队列
    Processor               *m_self{nullptr};       ///< 调度器所在的processor
    int                      m_min_threads{0};      ///< 最小调度线程数
    int                      m_max_threads{0};      ///< 最大调度线程数
    bool                     m_start{false};        ///< 启动标志
    std::size_t              m_ids{0};              ///< 用于给processor分配id

    Scheduler();
    ~Scheduler();
    Scheduler(Scheduler const&) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler& operator=(Scheduler const&) = delete;
    Scheduler& operator=(Scheduler &&) = delete;
};

}

#endif
