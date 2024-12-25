#pragma once
#include "stdafx.h"
#include "RelayPath.h"
class CDemand
{
public:
    CDemand(void);
    ~CDemand(void);
    CDemand(const CDemand& Demand);
    void operator=(const CDemand& Demand);

private:
    DEMANDID m_uiDemandID;	// 需求的唯一标识符
    NODEID m_uiSourceId;	// 需求的源节点ID
    NODEID m_uiSinkId;	// 需求的目标节点ID（汇节点）
    TIME m_dArriveTime;	// 需求到达网络的时间
    TIME m_dCompleteTime;	// 需求完成传输的时间
    VOLUME m_dVolume;	// 需求的总数据量
    VOLUME m_dRemainingVolume;	// 需求剩余未传输的数据量
    VOLUME m_dDeliveredVolume;	// 需求已成功传输的数据量
//    bool m_bRouted;	// 标志需求是否已被路由   无用
    bool m_bAllDelivered;	// 标志需求是否已全部传输完成

    bool m_bRouteFailed; //标志需求是否无法被路由



//    void SetRouted(bool routed);	// 设置需求是否已被路由
    void SetAllDelivered(bool delivered);	// 设置需求是否已全部传输完成

public:
    CRelayPath m_Path;	// 表示需求的传输路径，包括经过的节点和链路

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
    void ReduceVolume(VOLUME consumeVolume);	// 在传输过程中减少需求的剩余数据量

    VOLUME GetDeliveredVolume();
    void UpdateDeliveredVolume(VOLUME moreDelivered, TIME simTime);	// 更新需求的已传输量，并在传输完成时标记需求为已完成

//    bool GetRouted();
    bool GetAllDelivered();

    bool GetRoutedFailed();

    void InitRelayPath(list<NODEID>& nodeList, list<LINKID>& linkList);	// 初始化需求的传输路径，包括路径中的节点和链路
    void CheckRoutedFailed();  // 检查因故障而无法完成传输的demand并添加标记
};

