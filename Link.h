#pragma once
#include "KeyManager.h"
class CLink
{
public:
	CLink(void);
	~CLink(void);
	CLink(const CLink& Link);
	void operator=(const CLink& Link);


private:
	LINKID m_uiLinkId;	// ��·��Ψһ��ʶ��
	NODEID m_uiSourceId;	// ��·��Դ�ڵ�ID
	NODEID m_uiSinkId;	// ��·��Ŀ�Ľڵ�ID����ڵ㣩
	RATE m_dQKDRate;	// ��·��������Կ�ַ���QKD������
	TIME m_dDelay;	// ��·�Ĵ����ӳ�
	RATE m_dBandwidth;	// ��·�Ĵ���
	CKeyManager m_KeyManager;	// ����·���������Կ�����������ڹ�����·�ϵ���Կ�ַ�������


private://data structure for algorithms
	WEIGHT m_dWeight;	// ��·��Ȩ�أ�����·��ѡ���㷨��

public:
	list<DEMANDID> m_lCarriedDemands;	// һ������ID���б���ʾ��ǰ��·�����ڴ������������


	void SetLinkId(LINKID linkId);
	LINKID GetLinkId();

	void SetSourceId(NODEID sourceId);
	NODEID GetSourceId();

	void SetSinkId(NODEID sinkId);
	NODEID GetSinkId();

	void SetQKDRate(RATE QKDRate);
	RATE GetQKDRate();

	void SetLinkDelay(TIME delay);
	TIME GetLinkDelay();

	void SetBandwidth(RATE bandwidth);
	RATE GetBandwidth();

	void SetWeight(WEIGHT linkWeight);
	WEIGHT GetWeight();

	//key manager operations
	void ConsumeKeys(VOLUME keys);	// ������·��ָ����������Կ
	VOLUME GetAvaialbeKeys();	// ��ȡ��·�Ͽ��õ���Կ����
	void UpdateRemainingKeys(TIME executionTime);	// ����ִ��ʱ�������·��ʣ�����Կ��
};

