#include "StdAfx.h"
#include "InputOutput.h"


CInputOutput::CInputOutput(void)
{
}


CInputOutput::~CInputOutput(void)
{
}


CInputOutput::CInputOutput(CNetwork* Network)
{
	m_pNetwork=Network;
}

void CInputOutput::InitNodes(UINT nodeNum)
{
	// ���ݴ���Ľڵ����� nodeNum����������ڵ���� CNode �������� nodeId��Ȼ����Щ�ڵ���ӵ�������� m_pNetwork �Ľڵ��б� m_vAllNodes ��
	for (NODEID nodeId=0;nodeId<nodeNum;nodeId++)
	{
		CNode newNode;
		newNode.SetNodeId(nodeId);
		m_pNetwork->m_vAllNodes.push_back(newNode);
	}
}

void CInputOutput::InputNetworkInfo(string fileName)
{
	ifstream fin;
	fin.open(fileName.c_str());
	//read node info, here, we only need number of nodes
	UINT nodeNum;
	fin>>nodeNum;
	InitNodes(nodeNum);
	LINKID linkId;
	NODEID sourceId, sinkId;
	RATE keyRate;
	TIME proDelay;
	// ���ж�ȡ��·��Ϣ��������·ID��Դ�ڵ�ID��Ŀ��ڵ�ID��������Կ���� (keyRate) ����·�ӳ� (proDelay)
	while (fin>>linkId>>sourceId>>sinkId>>keyRate>>proDelay)
	{
		CLink newLink;
		newLink.SetLinkId(linkId);
		newLink.SetSourceId(sourceId);
		newLink.SetSinkId(sinkId);
		newLink.SetQKDRate(keyRate);
		newLink.SetLinkDelay(proDelay);
		m_pNetwork->m_vAllLinks.push_back(newLink);
		m_pNetwork->m_mNodePairToLink[make_pair(sourceId,sinkId)]=linkId;
		m_pNetwork->m_mNodePairToLink[make_pair(sinkId,sourceId)]=linkId;
		m_pNetwork->InitKeyManagerOverLink(linkId);
	}
	m_pNetwork->SetLinkNum(linkId+1);
	fin.close();
}

void CInputOutput::InputDemandInfo(string fileName)
{
	ifstream fin;
	fin.open(fileName.c_str());
	DEMANDID demandId;
	NODEID sourceId, sinkId;
	VOLUME demandVolume;
	TIME arriveTime;
	// ���ж�ȡ����ID��Դ�ڵ�ID��Ŀ��ڵ�ID�����������������󵽴�ʱ��
	while (fin>>demandId>>sourceId>>sinkId>>demandVolume>>arriveTime)
	{
		CDemand newDemand;
		newDemand.SetDemandId(demandId);
		newDemand.SetSourceId(sourceId);
		newDemand.SetSinkId(sinkId);
		newDemand.SetDemandVolume(demandVolume);
		newDemand.SetRemainingVolume(demandVolume);
		newDemand.SetArriveTime(arriveTime);
		newDemand.SetCompleteTime(INF);
		m_pNetwork->m_vAllDemands.push_back(newDemand);
		m_pNetwork->m_vAllNodes[sourceId].m_mRelayVolume[demandId]=demandVolume;
	}
}