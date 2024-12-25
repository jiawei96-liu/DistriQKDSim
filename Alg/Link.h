#pragma once
#include <random>
#include <ctime>
#include <unordered_set>
#include "stdafx.h"
#include "KeyManager.h"
class CLink
{
public:
    bool wait_or_not;    // 当前节点此时刻应该等待还是传输
    CLink(void);
    ~CLink(void);
    CLink(const CLink& Link);
    void operator=(const CLink& Link);


private:
    LINKID m_uiLinkId;	// 链路的唯一标识符
    NODEID m_uiSourceId;	// 链路的源节点ID
    NODEID m_uiSinkId;	// 链路的目的节点ID（汇节点）
    RATE m_dQKDRate;	// 链路的量子密钥分发（QKD）速率
    TIME m_dDelay;	// 链路的传输延迟
    RATE m_dBandwidth;	// 链路的带宽

    TIME m_dFaultTime;	// 链路故障的时间

    CKeyManager m_KeyManager;	// 与链路相关联的密钥管理器，用于管理链路上的密钥分发和消耗
    //TIME m_dLastSimTime = 0; // 用于记录上一次密钥更新的时间，初始化为0或其它适当初值


private://data structure for algorithms
    WEIGHT m_dWeight;	// 链路的权重，用于路径选择算法中

public:
//    list<DEMANDID> m_lCarriedDemands;	// 一个需求ID的列表，表示当前链路上正在传输的所有需求   ？？用法感觉不对
    unordered_set<DEMANDID> m_lCarriedDemands; // 我觉得应该是链路驱动传输，而不是节点驱动传输

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

    void SetFaultTime(TIME faultTime);
    TIME GetFaultTime();

    void SetKeyManager(const CKeyManager& keyManager);

    //key manager operations
    void ConsumeKeys(VOLUME keys);	// 消耗链路上指定数量的密钥
    VOLUME GetAvaialbeKeys();	// 获取链路上可用的密钥数量
    void UpdateRemainingKeys(TIME executionTime);	// 根据执行时间更新链路上剩余的密钥量
    void UpdateRemainingKeys(TIME executionTime, TIME m_dSimTime);	// 根据执行时间更新链路上剩余的密钥量
};

