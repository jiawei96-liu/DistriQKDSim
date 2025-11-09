#ifndef RESERVATION_STRATEGY_H
#define RESERVATION_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

/**
 * 端到端预约调度策略类
 * 该策略为指定节点调度中继需求，采用先到先服务的优先级策略，
 * 并实现端到端的密钥预约机制
 */
class ReservationStrategy : public SchedStrategy {
private:
    CNetwork *net;  // 网络实例指针

public:
    /**
     * 构造函数
     * @param _net 网络实例指针
     */
    ReservationStrategy(CNetwork* _net){ 
        net = _net; 
    }
    
    /**
     * 析构函数
     */
    ~ReservationStrategy() = default;

    /**
     * 调度算法主函数
     * 实现端到端预约调度：对于节点上的每个待处理需求，按到达时间优先，
     * 尝试在该需求的剩余路径上执行两阶段预约（按链路ID升序调用每条链路的TryReserve）。
     * 只有当路径上所有跳都能成功TryReserve时，才对所有跳调用CommitReservation
     * （并将该需求加入本次relayDemands以进行一次性端到端中继）。
     * 如果任意一跳失败，则释放已获得的临时预约并将该请求加入各跳的等待队列，等待后续重试。
     * 
     * @param nodeId 当前要调度的节点ID
     * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
     * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
     */
    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace sched

#endif // RESERVATION_STRATEGY_H