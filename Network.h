#pragma once
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
	vector<CNode> m_vAllNodes;	// �洢���������нڵ���б�
	vector<CLink> m_vAllLinks;	// �洢������������·���б�
	vector<CDemand> m_vAllDemands;	// �洢����������������б�
	map<pair<NODEID,NODEID>,LINKID> m_mNodePairToLink;	// ���ڸ��ݽڵ�Բ��Ҷ�Ӧ��·��ӳ���
	vector<CNetEvent> m_vAllExistingEvent;	// �洢�����е������¼�

	//data structure for simulation
	multimap<TIME,EVENTID> m_mUncompltedEvent;	// �洢δ����¼���ʱ����¼�ID��ӳ���
	vector<CKeyManager> m_vAllKeyManager;	// �洢������������Կ���������б�
	multimap<TIME,DEMANDID> m_mDemandArriveTime;	// �洢���󵽴�ʱ�������ID��ӳ���

private:
	UINT m_uiNodeNum;	// �����еĽڵ�����
	UINT m_uiLinkNum;	// �����е���·����
	UINT m_uiDemandNum;	// �����е���������
	TIME m_dSimTime;	// ��ǰģ��ʱ��

public:
	void SetNodeNum(UINT nodeNum);
	UINT GetNodeNum();

	void SetLinkNum(UINT linkNum);
	UINT GetLinkNum();

	void SetDemandNum(UINT demandNum);
	UINT GetDemandNum();

	TIME CurrentTime();	// ��ȡ��ǰģ��ʱ��
	void MoveSimTime(TIME executeTime);	// �ƽ�ģ��ʱ�䲢������Ӧ���¼�

	void InitKeyManagerOverLink(LINKID linkId);	// Ϊ�ض���·��ʼ����Կ������


public:
	//common algorithms
	bool ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList);	// ���ڼ����Դ�ڵ㵽��ڵ�����·�������ؾ����Ľڵ����·�б�


	//functions for relay routing	��ʼ��ָ�����������������м�·��
	void InitRelayPath(DEMANDID demandId);
	void InitRelayPath();//for all demands

	//function for scheduling
	TIME MinimumRemainingTimeFirst(NODEID nodeId, map<DEMANDID,VOLUME>& relayDemands);// ��������ڵ������ת��ִ��ʱ��
	TIME FindDemandToRelay(NODEID nodeId, map<DEMANDID,VOLUME>& relayDemand);	// ȷ��Ӧת�������󣬲����������ʱ��
	TIME FindDemandToRelay(map<NODEID,map<DEMANDID,VOLUME>>& relayDemand);
	void RelayForOneHop(TIME executeTime, map<NODEID,map<DEMANDID,VOLUME>>& relayDemands);// ִ��һ������ת���������м̵���һ��
	void UpdateRemainingKeys(TIME executionTime);	// ����·��ʣ�����Կ��
	void SimTimeForward(TIME executionTime);	// ��ģ��ʱ���ƽ�ָ����ִ��ʱ��

	//main process
	bool AllDemandsDelivered();	// ����Ƿ�������������ɴ���
	TIME OneTimeRelay();	// ִ��һ��ת�����������ƽ�ģ��ʱ��
	void MainProcess();	// ����ģ��������̣������ʼ��·������ִ������ת����ֱ�������������
};

