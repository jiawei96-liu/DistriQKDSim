#pragma once
#include "stdafx.h"
#include "Node.h"
#include "Link.h"
#include "Demand.h"
#include "NetEvent.h"
#include "Store.hpp"
#include "Web/dao/SimDao.hpp"
#include "Alg/Route/RouteFactory.h"
//#include "KeyManager.h"
#include <functional>
#include <condition_variable>
#include <mutex>

class CNetwork
{
public:
    CNetwork(void);
    ~CNetwork(void);
    //data structure for input
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
    TIME m_dSimTime;	// ��ǰģ��ʱ��
    UINT m_step;        // ִ�в���
    std::unique_ptr<route::RouteFactory> m_routeFactory;    //·�ɲ��Թ���
    std::unique_ptr<route::RouteStrategy> m_routeStrategy;  //��ǰ·�ɲ���
    SimResultStore simResStore;
    SimDao simDao;


public:
    void SetNodeNum(UINT nodeNum);
    UINT GetNodeNum();

    void SetLinkNum(UINT linkNum);
    UINT GetLinkNum();

    void SetDemandNum(UINT demandNum);
    UINT GetDemandNum();

    TIME CurrentTime();	// ��ȡ��ǰģ��ʱ��
    UINT CurrentStep();
    void MoveSimTime(TIME executeTime);	// �ƽ�ģ��ʱ�䲢������Ӧ���¼�

    void InitKeyManagerOverLink(LINKID linkId);	// Ϊ�ض���·��ʼ����Կ������

    void InitNodes(UINT nodeNum);

    void Clear();

    SimMetric getCurrentMetric();
    void StoreSimRes();
    void StoreSimResInDb();
    void beforeStore();

    void RunInBackGround();


    //common route algorithms
    std::function<bool(NODEID, NODEID, list<NODEID>&, list<LINKID>&)> currentRouteAlg;
    bool ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);	// ���ڼ����Դ�ڵ㵽��ڵ�����·�������ؾ����Ľڵ����·�б�
    bool Load_Balance(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);  // ���ؾ���·���㷨
    bool KeyRateShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);  // Ȩ��Ϊkeyrate�����·�㷨�����ؾ����Ľڵ����·�б�
    bool KeyRateShortestPathWithBinHeap(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) ; //�ö�����Ż���keyrate���·�㷨�����ؾ����Ľڵ����·�б�
    void ShowDemandPaths(); //�鿴���������demand��·����Ϣ���ڵ���Ϣ��
    //function for scheduling
    std::function<TIME(NODEID, map<DEMANDID, VOLUME>&)> currentScheduleAlg;
    TIME MinimumRemainingTimeFirst(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // ��������ڵ������ת��ִ��ʱ��
    // TIME MinimumRemainingTimeFirstLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands);
    TIME AverageKeyScheduling(NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands); // ��������ڵ������ת��ִ��ʱ��
    //link based������·
    TIME MinimumRemainingTimeFirstLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands);
    TIME FindDemandToRelayLinkBased(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand);


public:
    //functions for relay routing	��ʼ��ָ�����������������м�·��
    void InitRelayPath(DEMANDID demandId);
    void InitRelayPath();//for all demands

    // void InitRelayPath(size_t max_threads); // �������İ汾

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

