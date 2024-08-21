#pragma once
#include "RelayPath.h"
class CDemand
{
public:
	CDemand(void);
	~CDemand(void);
	CDemand(const CDemand& Demand);
	void operator=(const CDemand& Demand);

private:
	DEMANDID m_uiDemandID;	// �����Ψһ��ʶ��
	NODEID m_uiSourceId;	// �����Դ�ڵ�ID
	NODEID m_uiSinkId;	// �����Ŀ��ڵ�ID����ڵ㣩
	TIME m_dArriveTime;	// ���󵽴������ʱ��
	TIME m_dCompleteTime;	// ������ɴ����ʱ��
	VOLUME m_dVolume;	// �������������
	VOLUME m_dRemainingVolume;	// ����ʣ��δ�����������
	VOLUME m_dDeliveredVolume;	// �����ѳɹ������������
	bool m_bRouted;	// ��־�����Ƿ��ѱ�·��
	bool m_bAllDelivered;	// ��־�����Ƿ���ȫ���������


	void SetRouted(bool routed);	// ���������Ƿ��ѱ�·��
	void SetAllDelivered(bool delivered);	// ���������Ƿ���ȫ���������
public:
	CRelayPath m_Path;	// ��ʾ����Ĵ���·�������������Ľڵ����·

	DEMANDID GetDemandId();
	void SetDemandId(DEMANDID demandId);

	NODEID GetSourceId();
	void SetSourceId(NODEID sourceId);

	NODEID GetSinkId();
	void SetSinkId(NODEID sinkId);

	TIME GetArriveTime();
	void SetArriveTime(TIME arriveTime);

	TIME GetCompleteTime();
	void SetCompleteTime(TIME completeTime);

	VOLUME GetDemandVolume();
	void SetDemandVolume(VOLUME demandVolume);

	void SetRemainingVolume(VOLUME Volume);
	VOLUME GetRemainingVolume();
	void ReduceVolume(VOLUME consumeVolume);	// �ڴ�������м��������ʣ��������

	VOLUME GetDeliveredVolume();
	void UpdateDeliveredVolume(VOLUME moreDelivered, TIME simTime);	// ����������Ѵ����������ڴ������ʱ�������Ϊ�����

	bool GetRouted();
	bool GetAllDelivered();

	void InitRelayPath(list<NODEID>& nodeList, list<LINKID>& linkList);	// ��ʼ������Ĵ���·��������·���еĽڵ����·
};

