#include "MultiDomainRouteManager.h"
#include "Alg/Network.h"
#include "BgpStrategy.h"
#include <set>

using namespace route;

// 构造函数：初始化路由管理器，自动创建BGP策略对象
// net_：指向全局网络对象
MultiDomainRouteManager::MultiDomainRouteManager(CNetwork* net_) : net(net_) {
    // BGP策略对象只需一份，专用于跨域需求
    bgpStrategy = std::make_unique<BgpStrategy>(net);
}

// 析构函数：自动清理所有已创建的域内路由对象
MultiDomainRouteManager::~MultiDomainRouteManager() {
    ClearAllStrategies();
}

// 配置某个子域的路由类型（如OSPF、BFS、KeyRate等），支持动态切换
void MultiDomainRouteManager::SetDomainStrategy(int subdomainId, RouteStrategyType type) {
    domainStrategyMap[subdomainId] = type;
    // 清理旧对象，延迟重建，保证切换协议时生效
    domainStrategyObj.erase(subdomainId);
}

// 查询某个子域当前配置的路由类型，未配置则返回默认类型
RouteStrategyType MultiDomainRouteManager::GetDomainStrategy(int subdomainId) const {
    auto it = domainStrategyMap.find(subdomainId);
    return it != domainStrategyMap.end() ? it->second : defaultStrategy;
}

// 设置所有未显式配置子域的默认路由类型
void MultiDomainRouteManager::SetDefaultStrategy(RouteStrategyType type) {
    defaultStrategy = type;
}

// 获取当前默认路由类型
RouteStrategyType MultiDomainRouteManager::GetDefaultStrategy() const {
    return defaultStrategy;
}

// 获取所有已注册的子域ID集合
std::set<int> MultiDomainRouteManager::GetAllDomains() const {
    std::set<int> domains;
    for (const auto& kv : domainStrategyMap) domains.insert(kv.first);
    return domains;
}

// 清理所有已创建的域内路由对象（如需重置或析构时调用）
void MultiDomainRouteManager::ClearAllStrategies() {
    domainStrategyObj.clear();
}

// 注入/替换BGP策略对象（如需自定义BGP实现时使用）
void MultiDomainRouteManager::SetBgpStrategy(std::unique_ptr<RouteStrategy> bgp) {
    bgpStrategy = std::move(bgp);
}

// 确保某个子域的路由对象已创建，延迟初始化，避免重复创建
void MultiDomainRouteManager::ensureDomainStrategy(int subdomainId) {
    if (domainStrategyObj.count(subdomainId) == 0) {
        RouteStrategyType type = defaultStrategy;
        if (domainStrategyMap.count(subdomainId))
            type = domainStrategyMap[subdomainId];
        // 通过工厂创建对应类型的路由对象
        domainStrategyObj[subdomainId] = net->m_routeFactory->CreateStrategy(type);
    }
}

// 路由主入口：根据源/目的节点自动分流到域内协议或BGP
// sourceId, sinkId：需求的源/目的节点ID
// nodeList, linkList：输出路径节点序列和链路序列
// 返回值：true=路由成功，false=不可达
bool MultiDomainRouteManager::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // 获取源/目的节点所属子域
    int srcDomain = net->m_vAllNodes[sourceId].GetSubdomainId();
    int dstDomain = net->m_vAllNodes[sinkId].GetSubdomainId();
    if (srcDomain == dstDomain) {
        // 域内需求，自动调用对应子域协议
        ensureDomainStrategy(srcDomain);
        return domainStrategyObj[srcDomain]->Route(sourceId, sinkId, nodeList, linkList);
    } else {
        // 跨域需求，自动调用BGP
        return bgpStrategy ? bgpStrategy->Route(sourceId, sinkId, nodeList, linkList) : false;
    }
}
