#pragma once
#include "stdafx.h"
class CNetEvent
{
public:
    CNetEvent(void);
    ~CNetEvent(void);
    CNetEvent(const CNetEvent& netEvent);
    void operator=(const CNetEvent& netEvent);

private:
    EVENTTYPE m_enEventType;	// 事件的类型，可能代表不同种类的网络事件（例如需求到达、链路故障等）
    TIME m_dEventTime;	// 事件发生的时间
    DEMANDID m_uiAssociatedDemand;	// 与此事件相关联的需求ID

public:
    void SetEventType(EVENTTYPE eventType);
    EVENTTYPE GetEventType();

    void SetEventTime(TIME eventTime);
    TIME GetEventTime();

    void SetAssociateDemand(DEMANDID demandId);
    DEMANDID GetAssociateDemand();
};

