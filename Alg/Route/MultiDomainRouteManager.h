// MultiDomainRouteManager.h
#ifndef MULTIDOMAINROUTEMANAGER_H
#define MULTIDOMAINROUTEMANAGER_H

#include "RouteFactory.h"
#include <map>
#include <memory>
#include <list>
#include <set>
#include <mutex>

class CNetwork;

namespace route {

class MultiDomainRouteManager{
public:
    MultiDomainRouteManager(CNetwork* net);
    ~MultiDomainRouteManager();

    // 配置每个子域的路由类型
    void SetDomainStrategy(int subdomainId, RouteStrategyType type);
    // 获取子域当前协议
    RouteStrategyType GetDomainStrategy(int subdomainId) const;
    // 设置默认协议
    void SetDefaultStrategy(RouteStrategyType type);
    RouteStrategyType GetDefaultStrategy() const;
    // 获取所有子域ID
    std::set<int> GetAllDomains() const;
    // 清理所有策略对象
    void ClearAllStrategies();
    // 注入/重置BGP策略
    void SetBgpStrategy(std::unique_ptr<RouteStrategy> bgp);
    // 移除某个子域的策略配置和对象
    void RemoveDomainStrategy(int subdomainId);
    // 立即应用（重建）某子域的策略对象（强制重建）
    void ApplyDomainStrategyNow(int subdomainId,std::string soPath="");

    // 主调度接口：自动分流
    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList);

private:
    CNetwork* net;
    std::map<int, RouteStrategyType> domainStrategyMap; // 子域ID->路由类型
    std::map<int, std::unique_ptr<RouteStrategy>> domainStrategyObj; // 子域ID->路由对象
    std::unique_ptr<RouteStrategy> bgpStrategy; // BGP对象
    RouteStrategyType defaultStrategy = RouteType_Ospf;
    void ensureDomainStrategy(int subdomainId);
    // 保护并发访问
    mutable std::mutex mtx;
};

} // namespace route

#endif // MULTIDOMAINROUTEMANAGER_H
