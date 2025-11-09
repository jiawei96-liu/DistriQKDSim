#ifndef AVG_STRATEGY_H
#define AVG_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

/**
 * 平均密钥调度策略类
 * 该策略为指定节点调度中继需求，采用平均密钥量的调度策略
 */
class AvgStrategy : public SchedStrategy {
private:
    CNetwork *net;  // 网络实例指针

public:
    /**
     * 构造函数
     * @param _net 网络实例指针
     */
    AvgStrategy(CNetwork* _net){
        net = _net;
    }
    
    /**
     * 析构函数
     */
    ~AvgStrategy() = default;

    /**
     * 调度算法主函数
     * 平均密钥调度算法
     * 为指定节点调度中继需求，采用平均密钥量的调度策略
     * 
     * @param nodeId 当前要调度的节点ID
     * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
     * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
     */
    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace sched

#endif // AVG_STRATEGY_H