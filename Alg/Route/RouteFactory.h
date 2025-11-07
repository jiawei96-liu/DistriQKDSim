#ifndef ROUTE_FACTORY_H
#define ROUTE_FACTORY_H

#include <string>
#include <functional>
#include <unordered_map>
// #include "IRouteStrategy.h"

// namespace route {

// // 路由策略工厂类
// // 负责注册和创建所有支持的路由策略对象，支持按名称动态生成
// // 典型用法：
// //   RouteFactory::RegisterAll();
// //   auto* strategy = RouteFactory::Create("ospf");
// class RouteFactory {
// public:
//     using CreatorFunc = std::function<IRouteStrategy*()>;
//     // 注册所有支持的路由策略
//     void RegisterAll();
//     // 注册单个路由策略
//     void Register(const std::string& name, CreatorFunc func);
//     // 创建指定名称的路由策略对象
//     IRouteStrategy* Create(const std::string& name);
// private:
//     std::unordered_map<std::string, CreatorFunc> m_mCreators; // 策略名称到创建函数的映射
// };

// } // namespace route

// #endif // ROUTE_FACTORY_H
// #ifndef ROUTE_FACTORY_H
// #define ROUTE_FACTORY_H

#include<memory>
#include "Alg/stdafx.h"

class CNetwork;

namespace route
{

typedef enum {
    RouteType_Bfs,
    RouteType_KeyRateShortestPath,
    RouteType_Ospf,
    RouteType_Bgp,
    RouteType_Custom,
    RouteType_demo,
    RouteType_MultiDomain,
    RouteType_Unknown
} RouteStrategyType;

class RouteStrategy
{
private:
    /* data */
public:
    RouteStrategy(/* args */)=default;
    ~RouteStrategy()=default;
    virtual bool Route(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) =0;
};  

class RouteFactory
{
private:
    /* data */
    CNetwork * net;
public:
    RouteFactory(CNetwork * net);
    ~RouteFactory();
    std::unique_ptr<RouteStrategy> CreateStrategy(RouteStrategyType type);
    
    //load user strategy from .so file
    std::unique_ptr<RouteStrategy> CreateUserStrategy(std::string soPath);
};


} // namespace route

#endif
