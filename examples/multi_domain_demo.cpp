#include <iostream>
#include "Alg/Network.h"
#include "Alg/Route/MultiDomainRouteManager.h"

int main(){
    // 演示：如何通过 MultiDomainRouteManager 配置子域并查询路径。
    // 说明：为了示例简单，这里假设你已有一个可用的 CSV 文件
    // ../data/networks/network(500).csv 或者其他样例文件。

    CNetwork net;
    std::string csv = "../data/networks/network(500).csv";
    std::cout<<"Loading topology from: "<<csv<<"\n";
    net.LoadNetworkFullCSV(csv);

    if(!net.GetMultiDomainRouteMgr()){
        std::cerr<<"MultiDomainRouteMgr not initialized\n";
        return 1;
    }

    // 把子域 0 设置为 BFS，子域 1 设置为 KeyRate
    net.GetMultiDomainRouteMgr()->SetDomainStrategy(0, route::RouteType_Bfs);
    net.GetMultiDomainRouteMgr()->SetDomainStrategy(1, route::RouteType_KeyRateShortestPath);

    // 立即应用子域 0 的策略
    net.GetMultiDomainRouteMgr()->ApplyDomainStrategyNow(0);
    net.GetMultiDomainRouteMgr()->ApplyDomainStrategyNow(1);

    // 做一次路由查询示例（请替换为存在的节点id）
    NODEID src = 0;
    NODEID dst = 10;
    std::list<NODEID> nodeList;
    std::list<LINKID> linkList;
    bool ok = net.GetMultiDomainRouteMgr()->Route(src,dst,nodeList,linkList);
    if(!ok){
        std::cout<<"Route failed or not reachable\n";
        return 0;
    }
    std::cout<<"Route found: ";
    for(auto id: nodeList) std::cout<<id<<" ";
    std::cout<<"\n";

    return 0;
}
