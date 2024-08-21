#pragma once
class CNetEvent
{
public:
	CNetEvent(void);
	~CNetEvent(void);
	CNetEvent(const CNetEvent& netEvent);
	void operator=(const CNetEvent& netEvent);

private:
	EVENTTYPE m_enEventType;	// �¼������ͣ����ܴ���ͬ����������¼����������󵽴��·���ϵȣ�
	TIME m_dEventTime;	// �¼�������ʱ��
	DEMANDID m_uiAssociatedDemand;	// ����¼������������ID

public:
	void SetEventType(EVENTTYPE eventType);
	EVENTTYPE GetEventType();

	void SetEventTime(TIME eventTime);
	TIME GetEventTime();

	void SetAssociateDemand(DEMANDID demandId);
	DEMANDID GetAssociateDemand();
};

