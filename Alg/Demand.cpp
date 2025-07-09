#include "Demand.h"

CDemand::CDemand(void)
{
    m_dDeliveredVolume = 0;
//    m_bRouted = false;
    m_bRouteFailed = false;
    m_bAllDelivered = false;
    m_bRouteFailed = false;
}


CDemand::~CDemand(void)
{
}


CDemand::CDemand(const CDemand& Demand)
{
    m_uiDemandID = Demand.m_uiDemandID;
    m_uiSourceId = Demand.m_uiSourceId;
    m_uiSinkId = Demand.m_uiSinkId;
    m_dArriveTime = Demand.m_dArriveTime;
    m_dCompleteTime = Demand.m_dCompleteTime;
    m_dVolume = Demand.m_dVolume;
    m_dRemainingVolume = Demand.m_dRemainingVolume;
    m_Path = Demand.m_Path;
//    m_bRouted = Demand.m_bRouted;
    m_bRouteFailed = Demand.m_bRouteFailed;
    m_bAllDelivered = Demand.m_bAllDelivered;
    m_dDeliveredVolume = Demand.m_dDeliveredVolume;
    m_bRouteFailed = Demand.m_bRouteFailed;
}

void CDemand::operator=(const CDemand& Demand)
{
    m_uiDemandID = Demand.m_uiDemandID;
    m_uiSourceId = Demand.m_uiSourceId;
    m_uiSinkId = Demand.m_uiSinkId;
    m_dArriveTime = Demand.m_dArriveTime;
    m_dCompleteTime = Demand.m_dCompleteTime;
    m_dVolume = Demand.m_dVolume;
    m_dRemainingVolume = Demand.m_dRemainingVolume;
    m_Path = Demand.m_Path;
//    m_bRouted = Demand.m_bRouted;
    m_bRouteFailed = Demand.m_bRouteFailed;
    m_bAllDelivered = Demand.m_bAllDelivered;
    m_dDeliveredVolume = Demand.m_dDeliveredVolume;
    m_bRouteFailed = Demand.m_bRouteFailed;
}

void CDemand::SetDemandId(DEMANDID demandId)
{
    m_uiDemandID = demandId;
}

DEMANDID CDemand::GetDemandId()
{
    return m_uiDemandID;
}

void CDemand::SetSourceId(NODEID sourceId)
{
    m_uiSourceId = sourceId;
}


NODEID CDemand::GetSourceId()
{
    return m_uiSourceId;
}

void CDemand::SetSinkId(NODEID sinkId)
{
    m_uiSinkId = sinkId;
}

NODEID CDemand::GetSinkId()
{
    return m_uiSinkId;
}

TIME CDemand::GetArriveTime()
{
    return m_dArriveTime;
}

void CDemand::SetArriveTime(TIME arriveTime)
{
    m_dArriveTime = arriveTime;
}

TIME CDemand::GetCompleteTime()
{
    return m_dCompleteTime;
}

void CDemand::SetCompleteTime(TIME completeTime)
{
    m_dCompleteTime = completeTime;
}

VOLUME CDemand::GetDemandVolume()
{
    return m_dVolume;
}

void CDemand::SetDemandVolume(VOLUME demandVolume)
{
    m_dVolume = demandVolume;
}

void CDemand::SetRemainingVolume(VOLUME Volume)
{
    m_dRemainingVolume = Volume;
}

VOLUME CDemand::GetRemainingVolume()
{
    return m_dRemainingVolume;
}

void CDemand::ReduceVolume(VOLUME consumeVolume)
{
    // 减少剩余的数据量
    m_dRemainingVolume -= consumeVolume;
    if (m_dRemainingVolume < 0)
    {
        m_dRemainingVolume = 0;
    }
    // // 检查剩余的数据量是否小于一个非常小的负值（表示近似为负数）
    // if (m_dRemainingVolume < NEGSMALLVALUE)
    // {
    //     // 输出错误信息，指出哪个需求的剩余数据量变成了负值
    //     cout << "check why the volume of DEMAND " << m_uiDemandID << " become a negative value." << endl;

    //     // 暂停程序，让用户看到错误信息
    //     getchar();

    //     // 退出程序
    //     exit(0);
    // }
}

//void CDemand::SetRouted(bool routed)
//{
//    m_bRouted = routed;
//}

void CDemand::SetAllDelivered(bool delivered)
{
    m_bAllDelivered = delivered;
}

//bool CDemand::GetRouted()
//{
//    return m_bRouted;
//}

bool CDemand::GetRoutedFailed()
{
    return m_bRouteFailed;
}

VOLUME CDemand::GetDeliveredVolume()
{
    return m_dDeliveredVolume;
}

void CDemand::UpdateDeliveredVolume(VOLUME moreDelivered, TIME simTime)
{
    m_dDeliveredVolume += moreDelivered;
    if (m_dDeliveredVolume + SMALLNUM >= m_dVolume)
    {
        m_bAllDelivered = true;
        m_dCompleteTime = simTime;
    }
}

void CDemand::CheckRoutedFailed()
{
    m_bRouteFailed = true;
    m_bAllDelivered = true;
}

void CDemand::InitRelayPath(list<NODEID>& nodeList, list<LINKID>& linkList) //在CNetwork::InitRelayPath()中被调用
{
//    m_bRouted = true;
    m_Path.SetAssociateDemand(m_uiDemandID);
    m_Path.m_lTraversedNodes = nodeList;
    m_Path.m_lTraversedLinks = linkList;
    m_Path.SetSourceId(m_uiSourceId);
    m_Path.SetSinkId(m_uiSinkId);
//    list<NODEID>::iterator nodeIter;
    auto nodeIter = nodeList.begin();
    NODEID curNode = *nodeIter;
    nodeIter++;
    // 这里对m_Path.m_mNextNode的操作只有增加和修改，没有删除
    for (; nodeIter != nodeList.end(); nodeIter++)
    {
        m_Path.m_mNextNode[curNode] = *nodeIter;
        curNode = *nodeIter;
        // cout<<curNode<<'->';
    }
    // cout<<endl;
}

bool CDemand::GetAllDelivered()
{
    return m_bAllDelivered;
}


