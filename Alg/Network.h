#pragma once
#include "stdafx.h"
#include "Node.h"
#include "Link.h"
#include "Demand.h"
#include "NetEvent.h"
#include "Store.hpp"
#include "Alg/Route/RouteFactory.h"
#include "Alg/Sched/SchedFactory.h"
//#include "KeyManager.h"
#include <functional>
#include "Alg/Route/MultiDomainRouteManager.h"
#include <condition_variable>
#include <mutex>
#include <thread>

class SimDao;

class DomainInfo{
public:
    DomainInfo(int _nodeNum,int _edgeNum,int _gwNum,int _crossDomainEdgeNum):nodeNum(_nodeNum)
    ,edgeNum(_edgeNum),gateWayNum(_gwNum),crossDomainEdgeNum(_crossDomainEdgeNum){}
    ~DomainInfo()=default;
    int nodeNum;
    int edgeNum;
    int gateWayNum;
    int crossDomainEdgeNum;
};

class CNetwork
    
{
public:
// 新增：初始化节点和链路（带子域、网关属性）
    void LoadNetworkFullCSV(const std::string& filename);
    CNetwork(void);
    ~CNetwork(void);
    //data structure for input
    vector<DomainInfo> m_vDomainInfo;
    vector<CNode> m_vAllNodes;	// �洢���������нڵ���б�
    vector<CLink> m_vAllLinks;	// �洢������������·���б�
    vector<CDemand> m_vAllDemands;	// �洢����������������б�
    vector<CRelayPath> m_vAllRelayPaths;	// �洢�����������м�·�����б�
    map<pair<NODEID, NODEID>, LINKID> m_mNodePairToLink;	// ���ڸ��ݽڵ�Բ��Ҷ�Ӧ��·��ӳ���
    vector<CNetEvent> m_vAllExistingEvent;	// �洢�����е������¼�   ������δʹ��

    //data structure for simulation
    multimap<TIME, EVENTID> m_mUncompltedEvent;	// �洢δ����¼���ʱ����¼�ID��ӳ���  ������δʹ��

//    vector<CKeyManager> m_vAllKeyManager;	// �洢������������Կ���������б�

    multimap<TIME, DEMANDID> m_mDemandArriveTime;	// �洢���󵽴�ʱ�������ID��ӳ���    ����

    TIME FaultTime;  //��ʾ��ǰ���Ϸ�����ʱ��
    // vector<CLink> failedLink; //�洢��ǰʱ϶���ϵ�link
    list<LINKID> failedLink; //�洢��ǰʱ϶���ϵ�linkID

    uint32_t simID;
    string status;
    std::mutex mtx;
    std::condition_variable cv;


private:
    UINT m_uiNodeNum;	// �����еĽڵ�����
    UINT m_uiLinkNum;	// �����е���·����
    UINT m_uiDemandNum;	// �����е���������
public:
    TIME m_dSimTime;	// ��ǰģ��ʱ��
    UINT m_step;        // ִ�в���
private:
    std::unique_ptr<route::RouteFactory> m_routeFactory;    //·�ɲ��Թ���
    std::unique_ptr<route::RouteStrategy> m_routeStrategy;  //��ǰ·�ɲ���
    std::shared_ptr<route::MultiDomainRouteManager> m_multiDomainRouteMgr; // 多域路由控制器

    std::unique_ptr<sched::SchedFactory> m_schedFactory;    //·�ɲ��Թ���
    std::unique_ptr<sched::SchedStrategy> m_schedStrategy;  //��ǰ·�ɲ���
    SimResultStore simResStore;
    SimDao* simDao;
    
    // 记录已完成端到端预约、准备在下一步转发的需求集合
    std::unordered_set<DEMANDID> m_readyToRelay;


public:
    void SetNodeNum(UINT nodeNum);
    UINT GetNodeNum();

    void SetLinkNum(UINT linkNum);
    UINT GetLinkNum();

    void SetDemandNum(UINT demandNum);
    UINT GetDemandNum();

    TIME CurrentTime();	// ��ȡ��ǰģ��ʱ��
    UINT CurrentStep();
    void MoveSimTime(TIME executeTime);	// �ƽ�ģ��ʱ�������Ӧ���¼�

    void InitKeyManagerOverLink(LINKID linkId);	// Ϊ�ض���·��ʼ����Կ������

    void InitNodes(UINT nodeNum);

    void Clear();

    SimMetric getCurrentMetric();
    void StoreSimRes();
    void StoreSimResInDb();
    void beforeStore();

    void RunInBackGround();

    // Accessors for components that may be used by controllers or external modules
    route::RouteFactory* GetRouteFactory() { return m_routeFactory.get(); }
    route::RouteStrategy* GetRouteStrategy() { return m_routeStrategy.get(); }
    std::shared_ptr<route::MultiDomainRouteManager> GetMultiDomainRouteMgr() { return m_multiDomainRouteMgr; }

    //common route algorithms
    std::function<bool(NODEID, NODEID, list<NODEID>&, list<LINKID>&)> currentRouteAlg;
    bool ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);	// ���ڼ����Դ�ڵ㵽��ڵ�����·�������ؾ����Ľڵ����·�б�
    bool Load_Balance(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);  // ���ؾ���·���㷨
    bool KeyRateShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);  // Ȩ��Ϊkeyrate�����·�㷨�����ؾ����Ľڵ����·�б�
    bool KeyRateShortestPathWithBinHeap(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) ; //�ö�����Ż���keyrate���·�㷨�����ؾ����Ľڵ����·�б�
    void ShowDemandPaths(); //�鿴���������demand��·����Ϣ���ڵ���Ϣ��
    //function for scheduling
    // std::function<TIME(NODEID, map<DEMANDID, VOLUME>&)> currentScheduleAlg;
    std::function<TIME(LINKID, map<DEMANDID, VOLUME>&)> currentScheduleAlg;
    TIME MinimumRemainingTimeFirst(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // 计算给定节点的需求转发执行时间
    TIME AverageKeyScheduling(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // 计算给定节点的需求转发执行时间
    //link based基于链路
    TIME MinimumRemainingTimeFirstLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands);
    TIME AverageKeySchedulingLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands);
    TIME FindDemandToRelayLinkBased(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand);


public:
    //functions for relay routing	ʼָм·
    void InitRelayPath(DEMANDID demandId);
    void InitRelayPath();//for all demands

    // 添加动态路由计算方法，用于MinTime策略的"随到随算"
    void DynamicRouteDemand(DEMANDID demandId);

    void InitRelayPathByMultiThread(size_t max_threads=std::thread::hardware_concurrency()); // İ汾

    void InitLinkDemand();

    // functions for relay rerouting  ��������ʱ�Ŀ��٣���·�ɣ�����
    // void CheckFault(DEMANDID demandId);
    void CheckFault();
    void ReInitRelayPath(DEMANDID demandId);
    void ReInitRelayPath();//for all demands
    void Rerouting();

    TIME FindDemandToRelay(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemand);	// ȷ��Ӧת�������󣬲����������ʱ��
    TIME FindDemandToRelay(map<NODEID, map<DEMANDID, VOLUME>>& relayDemand);
    // TIME FindDemandToRelayLinkBased(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand);
    void RelayForOneHop(TIME executeTime, map<NODEID, map<DEMANDID, VOLUME>>& relayDemands); // ִ��һ������ת���������м̵���һ��
    void UpdateRemainingKeys(TIME executionTime);	// ������·��ʣ�����Կ��
    void UpdateRemainingKeys(TIME executionTime, TIME m_dSimTime);	// ������·��ʣ�����Կ��
        // retry reservations helpers (used when link key pools change)
        void ReattemptReservationsForLink(LINKID linkId);
        bool AttemptReserveForDemandFromNode(DEMANDID demandid, NODEID fromNode);
    void SimTimeForward(TIME executionTime);	// ��ģ��ʱ���ƽ�ָ����ִ��ʱ��

    //main process
    bool AllDemandsDelivered();	// ����Ƿ�������������ɴ���
    TIME OneTimeRelay();	// ִ��һ��ת�����������ƽ�ģ��ʱ��
//    void MainProcess();	// ����ģ��������̣������ʼ��·������ִ������ת����ֱ�������������   ����

    // �л��㷨
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
    void setdemoRoute(){
        m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_demo));
        // currentRouteAlg = [this](NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) -> bool
        // {
        //     return this->KeyRateShortestPathWithBinHeap(sourceId, sinkId, nodeList, linkList);
        // };
    }
    void setMinimumRemainingTimeFirst()
    {
        m_schedStrategy=std::move(m_schedFactory->CreateStrategy(sched::SchedType_Min));
        // currentScheduleAlg = [this](NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
        // {
            // return this->MinimumRemainingTimeFirst(nodeId, relayDemands);
        // };
    }
    void setAverageKeyScheduling()
    {
        m_schedStrategy=std::move(m_schedFactory->CreateStrategy(sched::SchedType_Avg));
        // currentScheduleAlg = [this](NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
        // {
        //     return this->AverageKeyScheduling(nodeId, relayDemands);
        // };
    }

    // 设置为端到端预约调度策略（两阶段预约 + 提交/释放）
    void setReservationSched()
    {
        m_schedStrategy=std::move(m_schedFactory->CreateStrategy(sched::SchedType_Reserve));
    }

    void setOspfRoute()
    {
        m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Ospf));
    }


    void setCustomRoute(){
        m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Custom));
    }

    void setCustomSched(){
        m_schedStrategy=std::move(m_schedFactory->CreateStrategy(sched::SchedType_Custom));
    }
};



