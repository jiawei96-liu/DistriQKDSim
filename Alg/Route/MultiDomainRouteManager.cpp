#include "MultiDomainRouteManager.h"
#include "Alg/Network.h"
#include "BgpStrategy.h"
#include <set>
#include <iostream>

using namespace route;

// 构造函数：初始化路由管理器，自动创建BGP策略对象
// net_：指向全局网络对象
MultiDomainRouteManager::MultiDomainRouteManager(CNetwork* net_) : net(net_) {
    // BGP策略对象只需一份，专用于跨域需求
    try {
        bgpStrategy = std::make_unique<BgpStrategy>(net);
    } catch (...) {
        std::cerr << "[MultiDomainRouteManager] Warning: failed to create default BGP strategy" << std::endl;
        bgpStrategy = nullptr;
    }
}

// 析构函数：自动清理所有已创建的域内路由对象
MultiDomainRouteManager::~MultiDomainRouteManager() {
    ClearAllStrategies();
}

// 配置某个子域的路由类型（如OSPF、BFS、KeyRate等），支持动态切换
void MultiDomainRouteManager::SetDomainStrategy(int subdomainId, RouteStrategyType type) {
    // std::lock_guard<std::mutex> lock(mtx);
    domainStrategyMap[subdomainId] = type;
    // 清理旧对象，延迟重建，保证切换协议时生效
    if (domainStrategyObj.count(subdomainId)) {
        domainStrategyObj.erase(subdomainId);
        std::cerr << "[MultiDomainRouteManager] Domain " << subdomainId << " strategy changed, old object removed" << std::endl;
    }
}

// 查询某个子域当前配置的路由类型，未配置则返回默认类型
RouteStrategyType MultiDomainRouteManager::GetDomainStrategy(int subdomainId) const {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = domainStrategyMap.find(subdomainId);
    return it != domainStrategyMap.end() ? it->second : defaultStrategy;
}

// 设置所有未显式配置子域的默认路由类型
void MultiDomainRouteManager::SetDefaultStrategy(RouteStrategyType type) {
    std::lock_guard<std::mutex> lock(mtx);
    defaultStrategy = type;
}

// 获取当前默认路由类型
RouteStrategyType MultiDomainRouteManager::GetDefaultStrategy() const {
    std::lock_guard<std::mutex> lock(mtx);
    return defaultStrategy;
}

// 获取所有已注册的子域ID集合
std::set<int> MultiDomainRouteManager::GetAllDomains() const {
    std::set<int> domains;
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& kv : domainStrategyMap) domains.insert(kv.first);
    return domains;
}

// 清理所有已创建的域内路由对象（如需重置或析构时调用）
void MultiDomainRouteManager::ClearAllStrategies() {
    std::lock_guard<std::mutex> lock(mtx);
    domainStrategyObj.clear();
    domainStrategyMap.clear();
    std::cerr << "[MultiDomainRouteManager] Cleared all domain strategies" << std::endl;
}

// 注入/替换BGP策略对象（如需自定义BGP实现时使用）
void MultiDomainRouteManager::SetBgpStrategy(std::unique_ptr<RouteStrategy> bgp) {
    std::lock_guard<std::mutex> lock(mtx);
    bgpStrategy = std::move(bgp);
    std::cerr << "[MultiDomainRouteManager] BGP strategy injected/updated" << std::endl;
}

void MultiDomainRouteManager::RemoveDomainStrategy(int subdomainId) {
    std::lock_guard<std::mutex> lock(mtx);
    domainStrategyMap.erase(subdomainId);
    if (domainStrategyObj.count(subdomainId)) {
        domainStrategyObj.erase(subdomainId);
    }
    std::cerr << "[MultiDomainRouteManager] Removed strategy for domain " << subdomainId << std::endl;
}

// 立即应用（重建）某子域的策略对象（强制重建）
void MultiDomainRouteManager::ApplyDomainStrategyNow(int subdomainId,std::string soPath) {
    // Force rebuild the strategy object
    {
        std::lock_guard<std::mutex> lock(mtx);
        domainStrategyObj.erase(subdomainId);
    }
    
    // create now
    RouteStrategyType type = defaultStrategy;
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (domainStrategyMap.count(subdomainId)) type = domainStrategyMap[subdomainId];
    }
    
    if (net && net->GetRouteFactory()) {
        std::unique_ptr<route::RouteStrategy> ptr;
        if(type<99){
            ptr = net->GetRouteFactory()->CreateStrategy(type);
        }else{
            ptr = net->GetRouteFactory()->CreateUserStrategy(soPath);
        }
        if (ptr) {
            std::lock_guard<std::mutex> lock(mtx);
            domainStrategyObj[subdomainId] = std::move(ptr);
            std::cerr << "[MultiDomainRouteManager] Applied strategy for domain " << subdomainId << std::endl;
        } else {
            std::cerr << "[MultiDomainRouteManager] Warning: factory failed to create strategy for domain " << subdomainId << std::endl;
        }
    } else {
        std::cerr << "[MultiDomainRouteManager] Error: network or routeFactory null when applying domain strategy" << std::endl;
    }
}

// !!! Dont use this in Route function, because it have mutex which will slow down the route progress !!!!!!
// !!! Dont use this in Route function, because it have mutex which will slow down the route progress !!!!!!
// !!! Dont use this in Route function, because it have mutex which will slow down the route progress !!!!!!
// 确保某个子域的路由对象已创建，延迟初始化，避免重复创建
void MultiDomainRouteManager::ensureDomainStrategy(int subdomainId) {
    // 先检查不加锁的情况下是否已经存在策略对象，避免不必要的加锁开销
    if (domainStrategyObj.count(subdomainId) > 0) {
        return; // 已经存在，直接返回
    }
    
    // 只有在确实需要创建策略对象时才加锁
    std::lock_guard<std::mutex> lock(mtx);
    // 再次检查，防止在获取锁的过程中其他线程已经创建了策略对象
    if (domainStrategyObj.count(subdomainId) > 0) {
        return;
    }
    
    RouteStrategyType type = defaultStrategy;
    if (domainStrategyMap.count(subdomainId)) type = domainStrategyMap[subdomainId];
    // 通过工厂创建对应类型的路由对象
    if (!net) {
        std::cerr << "[MultiDomainRouteManager] Error: network pointer is null" << std::endl;
        return;
    }
    if (!net->GetRouteFactory()) {
        std::cerr << "[MultiDomainRouteManager] Error: routeFactory is null" << std::endl;
        return;
    }
    try {
        auto ptr = net->GetRouteFactory()->CreateStrategy(type);
        if (ptr) domainStrategyObj[subdomainId] = std::move(ptr);
        else std::cerr << "[MultiDomainRouteManager] Warning: factory returned null for domain " << subdomainId << std::endl;
    } catch (...) {
        std::cerr << "[MultiDomainRouteManager] Exception when creating strategy for domain " << subdomainId << std::endl;
    }
}

// 路由主入口：根据源/目的节点自动分流到域内协议或BGP
// sourceId, sinkId：需求的源/目的节点ID
// nodeList, linkList：输出路径节点序列和链路序列
// 返回值：true=路由成功，false=不可达
bool MultiDomainRouteManager::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // 边界检查
    if (!net) {
        std::cerr << "[MultiDomainRouteManager] Error: network pointer is null" << std::endl;
        return false;
    }
    if (sourceId < 0 || sourceId >= (NODEID)net->m_vAllNodes.size() || sinkId < 0 || sinkId >= (NODEID)net->m_vAllNodes.size()) {
        std::cerr << "[MultiDomainRouteManager] Error: sourceId or sinkId out of range" << std::endl;
        return false;
    }

    int srcDomain = net->m_vAllNodes[sourceId].GetSubdomainId(); // 获取源节点所属子域ID
    int dstDomain = net->m_vAllNodes[sinkId].GetSubdomainId(); // 获取目的节点所属子域ID
    if (srcDomain == dstDomain) {
        // 域内需求，自动调用对应子域协议
        auto it = domainStrategyObj.find(srcDomain);
        if (it == domainStrategyObj.end() || !(it->second)) {
            // 如果策略对象不存在，尝试创建它
            RouteStrategyType type = defaultStrategy;
            if (domainStrategyMap.count(srcDomain)) type = domainStrategyMap[srcDomain];
            
            if (net && net->GetRouteFactory()) {
                std::unique_ptr<route::RouteStrategy> ptr = net->GetRouteFactory()->CreateStrategy(type);
                if (ptr) {
                    domainStrategyObj[srcDomain] = std::move(ptr);
                    std::cerr << "[MultiDomainRouteManager] Auto-created strategy for domain " << srcDomain << std::endl;
                } else {
                    std::cerr << "[MultiDomainRouteManager] Error: failed to auto-create strategy for domain " << srcDomain << std::endl;
                    return false;
                }
            } else {
                std::cerr << "[MultiDomainRouteManager] Error: network or routeFactory null when auto-creating domain strategy" << std::endl;
                return false;
            }
            // 更新迭代器
            it = domainStrategyObj.find(srcDomain);
        }
        
        return it->second->Route(sourceId, sinkId, nodeList, linkList);
    } else {
        // 跨域需求，自动调用BGP
        if (!bgpStrategy) {
            std::cerr << "[MultiDomainRouteManager] Error: BGP strategy not set" << std::endl;
            return false;
        }
        return bgpStrategy->Route(sourceId, sinkId, nodeList, linkList);
    }
}
