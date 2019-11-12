#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include <list>
#include <vector>
#include "processor.h"

using namespace std;

/**
 * 调度器类，单例模式
 */
class Scheduler
{
public:
    /**
     * 启动调度器
     * @param min: 最小调度线程数，为0时，设置为cpu核心数，
     * @param max: 最大调度线程数，为0时，设置为min，
     * @note 如果max大于min，有必要时会自动扩充调度线程数.
     */
    void start(int min = 1, int max = 0);
    /**
     * 偷取任务
     * @param p 试图从其他processor偷取任务的processor
     */
    list<Task *> stealTask(Processor *p);
    /**
     * 添加任务
     * @param fn  运行函数指针
     * @param args 函数参数
     * @note 新任务将会被随机加入到某个processor中
     */
    void addTask(TaskFn fn, void *args);
    /**
     * 唤醒所有等待的任务
     */
    void wakeupAll();
    /**
     * 打印统计信息
     */
    void showStatistics();
    /**
     * 获取调度器实体
     */
    static Scheduler &getInstance();
private:

    vector<Processor *> m_processors; ///< 所有processor队列
    Processor *m_self{nullptr};       ///< 调度器所在的processor
    int m_min_threads{0};             ///< 最小调度线程数
    int m_max_threads{0};             ///< 最大调度线程数
    static size_t m_ids;              ///< 用于给processor分配id

    Scheduler();
    ~Scheduler();
    Scheduler(Scheduler const&) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler& operator=(Scheduler const&) = delete;
    Scheduler& operator=(Scheduler &&) = delete;
};

#endif
