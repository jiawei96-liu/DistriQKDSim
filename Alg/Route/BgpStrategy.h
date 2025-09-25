
// BgpStrategy.h
#ifndef BGP_STRATEGY_H
#define BGP_STRATEGY_H

#include "RouteFactory.h"
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <map>

namespace route {

// BGP 路由策略类
// 用于实现跨子域（自治系统）路由，自动选择网关节点，支持多域网络的路径计算
// 典型用法：由 MultiDomainRouteManager 或 RouteFactory 创建并调用
class BgpStrategy : public RouteStrategy {
private:
    CNetwork* net; // 指向网络对象，提供节点/链路信息

public:
    // 构造函数，传入网络指针
    BgpStrategy(CNetwork* _net) { net = _net; }
    ~BgpStrategy() = default;

    // 路由主函数
    // 输入：sourceId 源节点ID，sinkId 目的节点ID
    // 输出：nodeList 路径节点序列，linkList 路径链路序列
    // 返回值：true=路由成功，false=不可达
    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route

#endif // BGP_STRATEGY_H
