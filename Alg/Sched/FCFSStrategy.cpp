#include "FCFSStrategy.h"
#include "Alg/Network.h"
#include <limits>
#include <algorithm>

using namespace sched;

FCFSStrategy::FCFSStrategy(CNetwork* _net) {
    net = _net;
}

/**
 * 先到先服务(FCFS)端到端密钥预约调度算法
 * 为指定节点调度中继需求，采用先到先服务的优先级策略，
 * 并实现端到端的密钥预约机制
 * 
 * @param nodeId 当前要调度的节点ID
 * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
 * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
 */
TIME FCFSStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
    TIME executeTime = INF;  // 执行时间，初始化为无穷大

    // 获取网络相关数据
    auto& m_vAllLinks = net->m_vAllLinks;          // 所有链路集合
    auto& m_dSimTime = net->m_dSimTime;            // 当前仿真时间
    auto& m_vAllDemands = net->m_vAllDemands;      // 所有需求集合
    auto& m_vAllNodes = net->m_vAllNodes;          // 所有节点集合

    // 获取当前节点上等待转发的需求（id -> remaining volume）
    auto& nodeRelayMap = m_vAllNodes[nodeId].m_mRelayVolume;
    if (nodeRelayMap.empty()) return INF;

    // 收集待处理需求并按到达时间排序（先到先服务）
    std::vector<DEMANDID> pending;
    for (auto& kv : nodeRelayMap) {
        DEMANDID d = kv.first;
        // 跳过未到达的需求
        if (m_vAllDemands[d].GetArriveTime() > m_dSimTime + SMALLNUM) continue;
        pending.push_back(d);
    }
    if (pending.empty()) return INF;

    // 按照到达时间排序（先到先服务优先级）
    std::sort(pending.begin(), pending.end(), [&](DEMANDID a, DEMANDID b){
        return m_vAllDemands[a].GetArriveTime() < m_vAllDemands[b].GetArriveTime();
    });

    // 用户可修改：可以调整需求处理顺序策略（如按优先级、需求量等）
    for (DEMANDID demandid : pending) {
        VOLUME required = m_vAllNodes[nodeId].m_mRelayVolume[demandid];
        if (required == 0) continue;

        // 构建剩余路径
        std::vector<LINKID> remainingLinks;
        NODEID cur = nodeId;
        bool path_ok = true;
        
        // 用户可修改：可以调整路径验证逻辑
        while (cur != m_vAllDemands[demandid].GetSinkId()) {
            auto& nextMap = m_vAllDemands[demandid].m_Path.m_mNextNode;
            if (!nextMap.count(cur)) { 
                path_ok = false; 
                break; 
            }
            NODEID nxt = nextMap[cur];
            auto it = net->m_mNodePairToLink.find(std::make_pair(cur, nxt));
            if (it == net->m_mNodePairToLink.end()) { 
                path_ok = false; 
                break; 
            }
            remainingLinks.push_back(it->second);
            cur = nxt;
            // 防止环路
            if (remainingLinks.size() > net->GetLinkNum()) { 
                path_ok = false; 
                break; 
            }
        }
        if (!path_ok || remainingLinks.empty()) continue;

        // 尝试两阶段预约（按链路ID升序锁定以避免死锁）
        std::vector<LINKID> lockOrder = remainingLinks;
        std::sort(lockOrder.begin(), lockOrder.end());
        std::vector<LINKID> reservedLinks;
        bool all_ok = true;
        RATE bottleneck_bw = std::numeric_limits<RATE>::max();

        // 用户可修改：可以调整预约策略（如优先预约某些链路）
        for (LINKID lid : lockOrder) {
            CLink& lk = m_vAllLinks[lid];
            // 检查链路是否故障
            if (lk.GetFaultTime() > 0 && lk.GetFaultTime() <= m_dSimTime) { 
                all_ok = false; 
                break; 
            }
            // 尝试预约密钥
            if (!lk.TryReserve(demandid, required, m_vAllDemands[demandid].GetArriveTime())) { 
                all_ok = false; 
                break; 
            }
            reservedLinks.push_back(lid);
            // 更新瓶颈带宽
            if (lk.GetBandwidth() < bottleneck_bw) 
                bottleneck_bw = lk.GetBandwidth();
        }

        // 用户可修改：可以调整提交或回滚策略
        if (all_ok) {
            // 所有链路预约成功，提交预约
            for (LINKID lid : reservedLinks) {
                CLink& lk = m_vAllLinks[lid];
                lk.CommitReservation(demandid);
                lk.wait_or_not = false;
            }
            // 添加到调度结果中
            relayDemands[demandid] = required;
            // 计算执行时间
            if (bottleneck_bw > 0 && bottleneck_bw < std::numeric_limits<RATE>::max()) {
                TIME t = static_cast<TIME>(required) / static_cast<TIME>(bottleneck_bw);
                if (t < executeTime) executeTime = t;
            }
        } else {
            // 预约失败，释放已获得的预约
            for (LINKID lid : reservedLinks) {
                m_vAllLinks[lid].ReleaseReservation(demandid);
            }
            // 将需求添加到所有相关链路的等待队列中
            for (LINKID lid : remainingLinks) {
                m_vAllLinks[lid].AddToWaitingQueue(demandid, required, m_vAllDemands[demandid].GetArriveTime(), nodeId);
            }
        }
    }
    
    // 更新链路的等待状态 - 用户可修改：可以调整链路状态更新策略
    for (auto& link : m_vAllLinks) {
        if (link.GetAvaialbeKeys() <= 0) {
            link.wait_or_not = true;
            //密钥不足，设置较高的协商带宽占用
            //------------------添加部分------------------
            // 用户可修改：可以调整带宽分配比例
            link.SetClassicalBandwidth(0.3); // 设置经典带宽为总带宽的30%
            link.SetNegotiationBandwidth(); // 设置密钥协商带宽为剩余带宽
            //----------------------------------------
        } else {
            link.wait_or_not = false;
            //密钥充足，设置较低的协商带宽占用
            //------------------添加部分------------------
            // 用户可修改：可以调整带宽分配比例
            link.SetClassicalBandwidth(0.5); // 设置经典带宽为总带宽的50%
            link.SetNegotiationBandwidth(); // 设置密钥协商带宽为剩余带宽
            //----------------------------------------
        }
    }
    
    // 如果没有预约成功，估算等待时间
    // 用户可修改：可以调整等待时间估算策略
    if (executeTime == INF) {
        for (DEMANDID demandid : pending) {
            VOLUME required = m_vAllNodes[nodeId].m_mRelayVolume[demandid];
            if (required == 0) continue;
            NODEID cur = nodeId;
            bool path_ok = true;
            RATE min_qkd_rate = std::numeric_limits<RATE>::max();
            
            // 用户可修改：可以调整路径检查逻辑
            while (cur != m_vAllDemands[demandid].GetSinkId()) {
                auto& nextMap = m_vAllDemands[demandid].m_Path.m_mNextNode;
                if (!nextMap.count(cur)) { path_ok = false; break; }
                NODEID nxt = nextMap[cur];
                auto it = net->m_mNodePairToLink.find(std::make_pair(cur, nxt));
                if (it == net->m_mNodePairToLink.end()) { path_ok = false; break; }
                LINKID lid = it->second;
                CLink& lk = m_vAllLinks[lid];
                RATE rate = lk.GetQKDRate();
                if (rate < min_qkd_rate) min_qkd_rate = rate;
                cur = nxt;
                if (min_qkd_rate == 0) { path_ok = false; break; }
            }
            if (!path_ok || min_qkd_rate <= 0) continue;
            TIME wait = static_cast<TIME>(required) / static_cast<TIME>(min_qkd_rate);
            if (wait < executeTime) executeTime = wait;
        }
    }

    // 输出调试信息
    if (executeTime != INF) {
        cout << "executeTime:" << executeTime << endl;
    }
    
    return executeTime;
}