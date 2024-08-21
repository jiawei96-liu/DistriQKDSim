#pragma once
#include "KeyManager.h"
class CNode
{
public:
	CNode(void);
	~CNode(void);
	CNode(const CNode& Node);
	void operator=(const CNode& Node);

private:
	NODEID m_uiNodeID;	// �ڵ��Ψһ��ʶ����ID��

public:
	list<NODEID> m_lAdjNodes;	// �뵱ǰ�ڵ����ڵ������ڵ��ID
	list<NODEID> m_lAdjLinks;	// �뵱ǰ�ڵ���������·��ID
	map<DEMANDID,VOLUME> m_mRelayVolume;	// ���ڼ�¼��Ҫͨ���˽ڵ�ת����������������DEMANDID����ʾ����ID����ֵ��VOLUME����ʾ����������

public:
	void SetNodeId(NODEID nodeId);
	NODEID GetNodeId();

};

