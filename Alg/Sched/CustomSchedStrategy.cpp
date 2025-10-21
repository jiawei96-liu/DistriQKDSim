#include "CustomSchedStrategy.h"
#include "Alg/Network.h"

using namespace sched;

extern "C" sched::CustomSchedStrategy* createSchedStrategy(CNetwork* net) {
    return new sched::CustomSchedStrategy(net);
}
TIME CustomSchedStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
    TIME executeTime = INF;                // 表示当前的最小执行时间
    map<LINKID, DEMANDID> scheduledDemand; // 记录每条链路上计划要转发的需求
    map<DEMANDID, TIME> executeTimeDemand; // 记录需求的执行时间
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(0, 1); // 定义一个分布，生成 0 或 1

    auto& m_vAllLinks=net->m_vAllLinks;
    auto& m_dSimTime=net->m_dSimTime;
    auto& m_vAllDemands=net->m_vAllDemands;
    auto& m_vAllNodes=net->m_vAllNodes;
    auto& m_mNodePairToLink=net->m_mNodePairToLink;
    // 生成随机数
    int tempWait = dis(gen);
    for (auto demandIter = m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[nodeId].m_mRelayVolume.end(); demandIter++)
    {
        DEMANDID selectedDemand = demandIter->first;
        if (m_vAllDemands[selectedDemand].GetArriveTime() > m_dSimTime + SMALLNUM)
        {
            continue;
        }
        // 根据链路的带宽 bandwidth 和需求的剩余数据量 demandIter->second，计算需求的执行时间，并更新最小执行时间 executeTime
        NODEID nextNode = m_vAllDemands[selectedDemand].m_Path.m_mNextNode[nodeId];
        LINKID midLink = m_mNodePairToLink[make_pair(nodeId, nextNode)];
        RATE bandwidth = m_vAllLinks[midLink].GetBandwidth();

        // 获取该链路上的可用密钥量
        VOLUME availableKeyVolume = m_vAllLinks[midLink].GetAvaialbeKeys();
        VOLUME actualTransmittableVolume = min(demandIter->second, availableKeyVolume);
        TIME demandExecuteTime = actualTransmittableVolume / bandwidth; 
        if (demandExecuteTime < 3)
        {
            if (tempWait == 1)
            {
                m_vAllLinks[midLink].wait_or_not = true;
                continue;
            }
        }
        if (demandExecuteTime < executeTime)
        {

            if (demandIter->second > availableKeyVolume)
            {
                m_vAllLinks[midLink].wait_or_not = true;
                continue;
            }
            else
            {
                m_vAllLinks[midLink].wait_or_not = false;
                executeTime = demandExecuteTime;
            }
        }
        executeTimeDemand[selectedDemand] = demandExecuteTime;
        if (scheduledDemand.find(midLink) == scheduledDemand.end())
        {
            scheduledDemand[midLink] = selectedDemand;
        }
        else
        {
            DEMANDID preDemand = scheduledDemand[midLink];
            if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand])
            {
                scheduledDemand[midLink] = selectedDemand;
            }
        }

    }
    for (auto scheduledIter = scheduledDemand.begin(); scheduledIter != scheduledDemand.end(); scheduledIter++)
    {
        RATE bandwidth = m_vAllLinks[scheduledIter->first].GetBandwidth();
        relayDemands[scheduledIter->second] = bandwidth * executeTime;
    }
    return executeTime;
}

