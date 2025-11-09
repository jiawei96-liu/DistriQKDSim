#ifndef MIN_STRATEGY_H
#define MIN_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

/**
 * 最小剩余时间优先调度策略类
 * 该策略为指定节点调度中继需求，采用最小剩余数据量优先的策略
 * 优先调度剩余数据量较少的需求，以减少等待时间
 */
class MinStrategy : public SchedStrategy {
private:
    CNetwork *net;  // 网络实例指针

public:
    /**
     * 构造函数
     * @param _net 网络实例指针
     */
    MinStrategy(CNetwork* _net){
        net = _net;
    }
    
    /**
     * 析构函数
     */
    ~MinStrategy() = default;

    /**
     * 调度算法主函数
     * 最小剩余时间优先调度算法
     * 为指定节点调度中继需求，采用最小剩余数据量优先的策略
     * 优先调度剩余数据量较少的需求，以减少等待时间
     * 
     * @param nodeId 当前要调度的节点ID
     * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
     * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
     */
    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace sched

#endif // MIN_STRATEGY_H