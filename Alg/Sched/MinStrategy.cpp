#include "MinStrategy.h"
#include "Alg/Network.h"

using namespace sched;
TIME MinStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
    TIME executeTime = INF;                // 表示当前的最小执行时间
    map<LINKID, DEMANDID> scheduledDemand; // 记录每条链路上计划要转发的需求
    map<DEMANDID, TIME> executeTimeDemand; // 记录需求的执行时间
    // 遍历当前节点 nodeId 上的所有需求（记录在 m_mRelayVolume 中），跳过尚未到达的需求（通过到达时间判断）
    // 创建一个随机数引擎
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
            // this demand has not arrived yet
            continue;
        }
        // 根据链路的带宽 bandwidth 和需求的剩余数据量 demandIter->second，计算需求的执行时间，并更新最小执行时间 executeTime
        NODEID nextNode = m_vAllDemands[selectedDemand].m_Path.m_mNextNode[nodeId];
        LINKID midLink = m_mNodePairToLink[make_pair(nodeId, nextNode)];
        RATE bandwidth = m_vAllLinks[midLink].GetBandwidth();

        // 获取该链路上的可用密钥量
        VOLUME availableKeyVolume = m_vAllLinks[midLink].GetAvaialbeKeys();
        // cout << "availableKeyVolume" << availableKeyVolume << endl;
        VOLUME actualTransmittableVolume = min(demandIter->second, availableKeyVolume);
        // cout<<"actualTransmittableVolume:"<<actualTransmittableVolume<<endl;
        // 根据链路的带宽和实际可传输的数据量，计算需求的执行时间，并更新最小执行时间 executeTime

        TIME demandExecuteTime = actualTransmittableVolume / bandwidth;
        // cout << "bandwidth:" << bandwidth << endl;
        // cout << "demandExecuteTime:" << actualTransmittableVolume / bandwidth << endl;
        
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
                // TIME waitTime = (demandIter->second - availableKeyVolume) / m_vAllLinks[midLink].GetQKDRate();
                // if (waitTime < executeTime)
                // {
                //     executeTime = waitTime;
                // }
                continue;
                // executeTime = (demandIter->second - availableKeyVolume) / m_vAllLinks[midLink].GetQKDRate();
            }
            else
            {
                m_vAllLinks[midLink].wait_or_not = false;
                executeTime = demandExecuteTime;
            }
        }
        // 该需求的执行时间
        executeTimeDemand[selectedDemand] = demandExecuteTime;

        // 如果该链路上还没有被调度的需求，将当前需求 selectedDemand 设置为该链路的调度需求。
        // if (availableKeyVolume >= demandIter->second || availableKeyVolume >= minAvailableKeyVolume)

        if (scheduledDemand.find(midLink) == scheduledDemand.end())
        {
            scheduledDemand[midLink] = selectedDemand;
        }
        else // 如果该链路已经有一个需求被调度，那么比较新需求和已调度需求的剩余数据量，选择数据量较少的需求作为调度对象（最小剩余时间优先）
        {
            DEMANDID preDemand = scheduledDemand[midLink];
            if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand])
                // if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand] && m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand] != 0)
            {
                scheduledDemand[midLink] = selectedDemand;
            }
        }

    }

    // 遍历所有被调度的需求，计算它们在执行时间内的转发数据量（带宽乘以执行时间），并将这些数据量记录在 relayDemands 中
    for (auto scheduledIter = scheduledDemand.begin(); scheduledIter != scheduledDemand.end(); scheduledIter++)
    {
        RATE bandwidth = m_vAllLinks[scheduledIter->first].GetBandwidth();
        // if (bandwidth * executeTime < 1)
        // {
        //     relayDemands[scheduledIter->second] = 0;
        // }
        // else
        // {
        //     relayDemands[scheduledIter->second] = bandwidth * executeTime;
        // }
        relayDemands[scheduledIter->second] = bandwidth * executeTime;
    }
    // if (executeTime == INF)
    // {
    //     executeTime = 5;
    // }

    // if(executeTime != INF)
    //     cout << "executeTime:" << executeTime << endl;

    return executeTime;
}

