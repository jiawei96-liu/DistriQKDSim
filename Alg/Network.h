﻿#pragma once
#include "StdAfx.h"
#include "Node.h"
#include "Link.h"
#include "Demand.h"
#include "NetEvent.h"
#include "KeyManager.h"
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

    multimap<TIME, DEMANDID> m_mDemandArriveTime;	// 存储需求到达时间和需求ID的映射表

private:
    UINT m_uiNodeNum;	// 网络中的节点数量
    UINT m_uiLinkNum;	// 网络中的链路数量
    UINT m_uiDemandNum;	// 网络中的需求数量
    TIME m_dSimTime;	// 当前模拟时间

public:
    void SetNodeNum(UINT nodeNum);
    UINT GetNodeNum();

    void SetLinkNum(UINT linkNum);
    UINT GetLinkNum();

    void SetDemandNum(UINT demandNum);
    UINT GetDemandNum();

    TIME CurrentTime();	// 获取当前模拟时间
    void MoveSimTime(TIME executeTime);	// 推进模拟时间并处理相应的事件

    void InitKeyManagerOverLink(LINKID linkId);	// 为特定链路初始化密钥管理器

    void InitNodes(UINT nodeNum);


public:
    //common algorithms
    bool ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);	// 用于计算从源节点到汇节点的最短路径，返回经过的节点和链路列表


    //functions for relay routing	初始化指定需求或所有需求的中继路径
    void InitRelayPath(DEMANDID demandId);
    void InitRelayPath();//for all demands

    // functions for relay rerouting  发生故障时的抗毁（重路由）功能
    // void CheckFault(DEMANDID demandId);
    bool CheckFault();
    void ReInitRelayPath(DEMANDID demandId);
    void ReInitRelayPath();//for all demands
    void Rerouting();

    //function for scheduling
    TIME MinimumRemainingTimeFirst(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // 计算给定节点的需求转发执行时间
	// TIME AverageKeyScheduling(NODEID nodeId, map<DEMANDID,VOLUME>& relayDemands);// 计算给定节点的需求转发执行时间
    TIME FindDemandToRelay(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemand);	// 确定应转发的需求，并计算所需的时间
    TIME FindDemandToRelay(map<NODEID, map<DEMANDID, VOLUME>>& relayDemand);
    void RelayForOneHop(TIME executeTime, map<NODEID, map<DEMANDID, VOLUME>>& relayDemands); // 执行一次需求转发操作，中继到下一跳
    void UpdateRemainingKeys(TIME executionTime);	// 更新链路上剩余的密钥量
    void UpdateRemainingKeys(TIME executionTime, TIME m_dSimTime);	// 更新链路上剩余的密钥量
    void SimTimeForward(TIME executionTime);	// 将模拟时间推进指定的执行时间

    //main process
    bool AllDemandsDelivered();	// 检查是否所有需求都已完成传输
    TIME OneTimeRelay();	// 执行一次转发操作，并推进模拟时间
    void MainProcess();	// 网络模拟的主流程，负责初始化路径，逐步执行需求转发，直到所有需求完成
};

