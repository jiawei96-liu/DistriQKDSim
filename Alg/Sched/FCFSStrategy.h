#ifndef FCFS_STRATEGY_H
#define FCFS_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

/**
 * 先到先服务(FCFS)端到端密钥预约调度策略类
 * 该策略为指定节点调度中继需求，采用先到先服务的优先级策略，
 * 并实现端到端的密钥预约机制
 */
class FCFSStrategy : public SchedStrategy {
private:
    CNetwork *net;  // 网络实例指针

public:
    /**
     * 构造函数
     * @param _net 网络实例指针
     */
    FCFSStrategy(CNetwork* _net){
        net = _net;
    }
    
    /**
     * 析构函数
     */
    ~FCFSStrategy() = default;

    /**
     * 调度算法主函数
     * 为指定节点调度中继需求，采用先到先服务的优先级策略，考虑端到端路径的密钥可用性
     * 
     * @param nodeId 当前要调度的节点ID
     * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
     * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
     */
    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace sched

#endif // FCFS_STRATEGY_H