
// OspfStrategy.h
#ifndef OSPF_STRATEGY_H
#define OSPF_STRATEGY_H

#include "RouteFactory.h"
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <map>

namespace route {

// OSPF 路由策略类
// 实现基于 Dijkstra 算法的最短路径搜索，适用于单域内链路状态路由
// 典型用法：由 RouteFactory 或 MultiDomainRouteManager 创建并调用
class OspfStrategy : public RouteStrategy {
private:
    CNetwork* net; // 指向网络对象，提供节点/链路信息

public:
    // 构造函数，传入网络指针
    OspfStrategy(CNetwork* _net) {
        net = _net;
    }
    ~OspfStrategy() = default;

    // 路由主函数
    // 输入：sourceId 源节点ID，sinkId 目的节点ID
    // 输出：nodeList 路径节点序列，linkList 路径链路序列
    // 返回值：true=路由成功，false=不可达
    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route

#endif // OSPF_STRATEGY_H