#include "MinStrategy.h"
#include "Alg/Network.h"

using namespace sched;

/**
 * 最小剩余时间优先调度算法
 * 为指定节点调度中继需求，采用最小剩余数据量优先的策略
 * 优先调度剩余数据量较少的需求，以减少等待时间
 * 
 * @param nodeId 当前要调度的节点ID
 * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
 * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
 */
TIME MinStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
    TIME executeTime = INF;                // 表示当前的最小执行时间
    map<LINKID, DEMANDID> scheduledDemand; // 记录每条链路上计划要转发的需求
    map<DEMANDID, TIME> executeTimeDemand; // 记录需求的执行时间
    
    // 创建一个随机数引擎 - 用户可修改：可以更换随机数生成算法或调整决策逻辑
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(0, 1); // 定义一个分布，生成 0 或 1

    // 获取网络相关数据
    auto& m_vAllLinks=net->m_vAllLinks;
    auto& m_dSimTime=net->m_dSimTime;
    auto& m_vAllDemands=net->m_vAllDemands;
    auto& m_vAllNodes=net->m_vAllNodes;
    auto& m_mNodePairToLink=net->m_mNodePairToLink;
    
    // 生成随机数
    int tempWait = dis(gen);
    
    // 遍历当前节点 nodeId 上的所有需求（记录在 m_mRelayVolume 中），跳过尚未到达的需求（通过到达时间判断）
    for (auto demandIter = m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[nodeId].m_mRelayVolume.end(); demandIter++)
    {
        DEMANDID selectedDemand = demandIter->first;
        // 跳过未到达的需求 - 用户可修改：可以调整需求到达时间判断条件
        if (m_vAllDemands[selectedDemand].GetArriveTime() > m_dSimTime + SMALLNUM)
        {
            // this demand has not arrived yet
            continue;
        }
        
        // 根据链路的带宽 bandwidth 和需求的剩余数据量 demandIter->second，计算需求的执行时间，并更新最小执行时间 executeTime
        NODEID nextNode = m_vAllDemands[selectedDemand].m_Path.m_mNextNode[nodeId];
        LINKID midLink = m_mNodePairToLink[make_pair(nodeId, nextNode)];
        
        //-------------------------修改部分-------------------------
        //RATE bandwidth = link.GetBandwidth();
        RATE classicalbandwidth = m_vAllLinks[midLink].GetClassicalBandwidth();  // 修正：使用正确的链路对象
        RATE negotiationbandwidth = m_vAllLinks[midLink].GetNegotiationBandwidth();
        //--------------------------------------------------

        // 获取该链路上的可用密钥量
        VOLUME availableKeyVolume = m_vAllLinks[midLink].GetAvaialbeKeys();
        // cout << "availableKeyVolume" << availableKeyVolume << endl;
        
        // 计算实际可传输的数据量（取需求剩余量和可用密钥量的最小值）
        VOLUME actualTransmittableVolume = min(demandIter->second, availableKeyVolume);
        // cout<<"actualTransmittableVolume:"<<actualTransmittableVolume<<endl;
        
        // 根据链路的带宽和实际可传输的数据量，计算需求的执行时间，并更新最小执行时间 executeTime
        TIME demandExecuteTime = actualTransmittableVolume / classicalbandwidth;
        // cout << "bandwidth:" << bandwidth << endl;
        // cout << "demandExecuteTime:" << actualTransmittableVolume / bandwidth << endl;
        
        // 最小执行时间阈值判断 - 用户可修改：可以调整阈值和等待逻辑
        if (demandExecuteTime < 3)  // 3是可调整的阈值
        {
            if (tempWait == 1)  // 随机等待决策 - 用户可修改：可以调整等待策略
            {
                m_vAllLinks[midLink].wait_or_not = true;
                continue;
            }
        }
        
        if (demandExecuteTime < executeTime)
        {
            // 密钥充足性判断和带宽分配 - 用户可修改：可以调整阈值和带宽分配策略
            if (demandIter->second > availableKeyVolume)
            {
                // 密钥不足的情况
                m_vAllLinks[midLink].wait_or_not = true;
                // ------------------添加部分------------------
                // 用户可修改：可以调整带宽分配比例
                m_vAllLinks[midLink].SetClassicalBandwidth(0.3); // 设置经典带宽为总带宽的30%
                m_vAllLinks[midLink].SetNegotiationBandwidth(); // 设置密钥协商带宽为剩余带宽
                //----------------------------------------
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
                // 密钥充足的情况
                m_vAllLinks[midLink].wait_or_not = false;
                // ------------------添加部分------------------
                // 用户可修改：可以调整带宽分配比例
                m_vAllLinks[midLink].SetClassicalBandwidth(0.7); // 设置经典带宽为总带宽的70% 修正：注释中应该是70%
                m_vAllLinks[midLink].SetNegotiationBandwidth(); // 设置密钥协商带宽为剩余带宽
                //----------------------------------------
                executeTime = demandExecuteTime;
            }
        }
        
        // 该需求的执行时间
        executeTimeDemand[selectedDemand] = demandExecuteTime;

        // 链路调度需求选择策略 - 用户可修改：可以调整调度选择算法
        // 如果该链路上还没有被调度的需求，将当前需求 selectedDemand 设置为该链路的调度需求。
        // if (availableKeyVolume >= demandIter->second || availableKeyVolume >= minAvailableKeyVolume)

        if (scheduledDemand.find(midLink) == scheduledDemand.end())
        {
            scheduledDemand[midLink] = selectedDemand;
        }
        else // 如果该链路已经有一个需求被调度，那么比较新需求和已调度需求的剩余数据量，选择数据量较少的需求作为调度对象（最小剩余时间优先）
        {
            DEMANDID preDemand = scheduledDemand[midLink];
            // 用户可修改：可以调整需求选择条件
            if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand])
                // if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand] && m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand] != 0)
            {
                scheduledDemand[midLink] = selectedDemand;
            }
        }
    }

    // 资源分配策略 - 用户可修改：可以调整数据量分配计算方式
    // 遍历所有被调度的需求，计算它们在执行时间内的转发数据量（带宽乘以执行时间），并将这些数据量记录在 relayDemands 中
    for (auto scheduledIter = scheduledDemand.begin(); scheduledIter != scheduledDemand.end(); scheduledIter++)
    {
        RATE bandwidth = m_vAllLinks[scheduledIter->first].GetBandwidth();
        // 用户可修改：可以调整数据量分配逻辑
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
    
    // 用户可修改：可以调整默认执行时间设置
    // if (executeTime == INF)
    // {
    //     executeTime = 5;
    // }

    // 输出调试信息
    // if(executeTime != INF)
    //     cout << "executeTime:" << executeTime << endl;

    return executeTime;
}