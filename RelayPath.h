#pragma once
class CRelayPath
{
public:
	CRelayPath(void);
	~CRelayPath(void);
	CRelayPath(const CRelayPath& path);
	void operator=(const CRelayPath& path);

private:
	NODEID m_uiSourceId;	// ·����Դ�ڵ�ID
	NODEID m_uiSinkId;	// ·����Ŀ�Ľڵ�ID����ڵ㣩
	DEMANDID m_uiAssociateDemand;	// ���·������������ID

public:
	list<NODEID> m_lTraversedNodes;	// һ���ڵ�ID���б���ʾ·�������ξ����Ľڵ�
	list<LINKID> m_lTraversedLinks;	// һ����·ID���б���ʾ·�������ξ�������·
	map<NODEID,NODEID> m_mNextNode;	// һ���ڵ�ID����һ���ڵ�ID��ӳ�䣬���ڿ��ٲ���·����ĳ�ڵ����һ���ڵ�

public:
	void SetSourceId(NODEID SourceId);
	NODEID GetSourceId();

	void SetSinkId(NODEID sinkId);
	NODEID GetSinkId();

	void SetAssociateDemand(DEMANDID demandId);
	DEMANDID GetAssocaiteDemand();
};

