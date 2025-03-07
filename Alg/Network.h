#pragma once
#include "stdafx.h"
#include "Node.h"
#include "Link.h"
#include "Demand.h"
#include "NetEvent.h"
#include "Alg/Route/RouteFactory.h"
//#include "KeyManager.h"
#include <functional>

class CNetwork
{
public:
    CNetwork(void);
    ~CNetwork(void);
    //data structure for input
    vector<CNode> m_vAllNodes;	// 存储网络中所有节点的列表
    vector<CLink> m_vAllLinks;	// 存储网络中所有链路的列表
    vector<CDemand> m_vAllDemands;	// 存储网络中所有需求的列表
    vector<CRelayPath> m_vAllRelayPaths;	// 存储网络中所有中继路径的列表
    map<pair<NODEID, NODEID>, LINKID> m_mNodePairToLink;	// 用于根据节点对查找对应链路的映射表
    vector<CNetEvent> m_vAllExistingEvent;	// 存储网络中的所有事件   保留，未使用

    //data structure for simulation
    multimap<TIME, EVENTID> m_mUncompltedEvent;	// 存储未完成事件的时间和事件ID的映射表  保留，未使用

//    vector<CKeyManager> m_vAllKeyManager;	// 存储网络中所有密钥管理器的列表

    multimap<TIME, DEMANDID> m_mDemandArriveTime;	// 存储需求到达时间和需求ID的映射表    有序

    TIME FaultTime;  //表示当前故障发生的时间
    // vector<CLink> failedLink; //存储当前时隙故障的link
    list<LINKID> failedLink; //存储当前时隙故障的linkID


private:
    UINT m_uiNodeNum;	// 网络中的节点数量
    UINT m_uiLinkNum;	// 网络中的链路数量
    UINT m_uiDemandNum;	// 网络中的需求数量
    TIME m_dSimTime;	// 当前模拟时间
    UINT m_step;        // 执行步数
    std::unique_ptr<route::RouteFactory> m_routeFactory;    //路由策略工厂
    std::unique_ptr<route::RouteStrategy> m_routeStrategy;  //当前路由策略

public:
    void SetNodeNum(UINT nodeNum);
    UINT GetNodeNum();

    void SetLinkNum(UINT linkNum);
    UINT GetLinkNum();

    void SetDemandNum(UINT demandNum);
    UINT GetDemandNum();

    TIME CurrentTime();	// 获取当前模拟时间
    UINT CurrentStep();
    void MoveSimTime(TIME executeTime);	// 推进模拟时间并处理相应的事件

    void InitKeyManagerOverLink(LINKID linkId);	// 为特定链路初始化密钥管理器

    void InitNodes(UINT nodeNum);

    void Clear();




    //common route algorithms
    std::function<bool(NODEID, NODEID, list<NODEID>&, list<LINKID>&)> currentRouteAlg;
    bool ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);	// 用于计算从源节点到汇节点的最短路径，返回经过的节点和链路列表
    bool Load_Balance(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);  // 负载均衡路由算法
    bool KeyRateShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);  // 权重为keyrate的最短路算法，返回经过的节点和链路列表
    bool KeyRateShortestPathWithBinHeap(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) ; //用二叉堆优化的keyrate最短路算法，返回经过的节点和链路列表
    void ShowDemandPaths(); //查看并输出所有demand的路径信息（节点信息）
    //function for scheduling
    std::function<TIME(NODEID, map<DEMANDID, VOLUME>&)> currentScheduleAlg;
    TIME MinimumRemainingTimeFirst(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // 计算给定节点的需求转发执行时间
    // TIME MinimumRemainingTimeFirstLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands);
    TIME AverageKeyScheduling(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // 计算给定节点的需求转发执行时间
    //link based基于链路
    TIME FindDemandToRelayLinkBased(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand);


public:
    //functions for relay routing	初始化指定需求或所有需求的中继路径
    void InitRelayPath(DEMANDID demandId);
    // void InitRelayPath();//for all demands

    void InitRelayPath(size_t max_threads); // 带参数的版本

    void InitLinkDemand();

    // functions for relay rerouting  发生故障时的抗毁（重路由）功能
    // void CheckFault(DEMANDID demandId);
    void CheckFault();
    void ReInitRelayPath(DEMANDID demandId);
    void ReInitRelayPath();//for all demands
    void Rerouting();

    TIME FindDemandToRelay(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemand);	// 确定应转发的需求，并计算所需的时间
    TIME FindDemandToRelay(map<NODEID, map<DEMANDID, VOLUME>>& relayDemand);
    // TIME FindDemandToRelayLinkBased(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand);
    void RelayForOneHop(TIME executeTime, map<NODEID, map<DEMANDID, VOLUME>>& relayDemands); // 执行一次需求转发操作，中继到下一跳
    void UpdateRemainingKeys(TIME executionTime);	// 更新链路上剩余的密钥量
    void UpdateRemainingKeys(TIME executionTime, TIME m_dSimTime);	// 更新链路上剩余的密钥量
    void SimTimeForward(TIME executionTime);	// 将模拟时间推进指定的执行时间

    //main process
    bool AllDemandsDelivered();	// 检查是否所有需求都已完成传输
    TIME OneTimeRelay();	// 执行一次转发操作，并推进模拟时间
//    void MainProcess();	// 网络模拟的主流程，负责初始化路径，逐步执行需求转发，直到所有需求完成   废弃

    // 切换算法
    void setShortestPath()
    {
        m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Bfs));
        // currentRouteAlg = [this](NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) -> bool
        // {
        //     return this->ShortestPath(sourceId, sinkId, nodeList, linkList);
        // };
    }
    void setKeyRateShortestPath()
    {
        m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_KeyRateShortestPath));
        // currentRouteAlg = [this](NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) -> bool
        // {
        //     return this->KeyRateShortestPath(sourceId, sinkId, nodeList, linkList);
        // };
    }
    void setKeyRateShortestPathWithBinHeap(){
        m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_KeyRateShortestPath));
        // currentRouteAlg = [this](NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) -> bool
        // {
        //     return this->KeyRateShortestPathWithBinHeap(sourceId, sinkId, nodeList, linkList);
        // };
    }
    void setMinimumRemainingTimeFirst()
    {
        currentScheduleAlg = [this](NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
        {
            return this->MinimumRemainingTimeFirst(nodeId, relayDemands);
        };
    }
    void setAverageKeyScheduling()
    {
        currentScheduleAlg = [this](NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
        {
            return this->AverageKeyScheduling(nodeId, relayDemands);
        };
    }
};

