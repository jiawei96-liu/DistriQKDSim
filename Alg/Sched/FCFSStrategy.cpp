#include "Alg/Network.h"
#include <vector>
#include <algorithm>
#include <map>
#include <mutex>

using namespace sched;

// 本实现采用“端到端预约”策略：
// 对于节点 nodeId 上的每个待转发需求，按到达时间（先到先服务）尝试为该需求在其剩余路径上
// 预约（占用）所需的全部密钥；只有当路径上每跳的可用密钥>=需求所需密钥时，才为该需求一次性
// 扣减各跳密钥并将该需求加入本次 relayDemands（表示端到端一次性中继）；当资源不足时，按优先级
// 分配并返回最小的执行时间（以便仿真推进）。

TIME AvgStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
    TIME executeTime = INF;

    auto &links = net->m_vAllLinks;
    auto &nodes = net->m_vAllNodes;
    auto &demands = net->m_vAllDemands;
    TIME now = net->m_dSimTime;

    // 获取当前节点上等待转发的需求（id -> remaining volume）
    auto &nodeRelayMap = nodes[nodeId].m_mRelayVolume;
    if (nodeRelayMap.empty()) return INF;

    // 收集待处理需求并按到达时间排序（先到先服务）
    std::vector<DEMANDID> pending;
    for (auto &kv : nodeRelayMap) {
        DEMANDID d = kv.first;
        // 跳过还未到达的需求（以防）
        if (demands[d].GetArriveTime() > now + SMALLNUM) continue;
        pending.push_back(d);
    }
    if (pending.empty()) return INF;

    std::sort(pending.begin(), pending.end(), [&](DEMANDID a, DEMANDID b){
        return demands[a].GetArriveTime() < demands[b].GetArriveTime();
    });

    // For each pending demand (in priority order), attempt to reserve keys on its remaining path
    for (DEMANDID demandid : pending) {
        VOLUME required = nodes[nodeId].m_mRelayVolume[demandid];
        if (required == 0) continue;

        // build list of link ids from current node to sink following demand path
        std::vector<LINKID> remainingLinks;
        NODEID cur = nodeId;
        bool path_ok = true;
        while (cur != demands[demandid].GetSinkId()) {
            auto &nextMap = demands[demandid].m_Path.m_mNextNode;
            if (!nextMap.count(cur)) { path_ok = false; break; }
            NODEID nxt = nextMap[cur];
            auto it = net->m_mNodePairToLink.find(std::make_pair(cur, nxt));
            if (it == net->m_mNodePairToLink.end()) { path_ok = false; break; }
            remainingLinks.push_back(it->second);
            cur = nxt;
            if (remainingLinks.size() > net->GetLinkNum()) { path_ok = false; break; }
        }
        if (!path_ok || remainingLinks.empty()) continue;

        // Try two-phase reservation: attempt TryReserve on each hop (in increasing link id order)
        std::vector<LINKID> lockOrder = remainingLinks;
        std::sort(lockOrder.begin(), lockOrder.end());
        std::vector<LINKID> reservedLinks;
        bool all_ok = true;
        RATE bottleneck_bw = std::numeric_limits<RATE>::max();

        // Attempt to reserve on each link one by one (locks inside TryReserve protect per-link state)
        for (LINKID lid : lockOrder) {
            CLink &lk = links[lid];
            // skip failed link
            if (lk.GetFaultTime() > 0 && lk.GetFaultTime() <= now) { all_ok = false; break; }
            if (!lk.TryReserve(demandid, required, demands[demandid].GetArriveTime())) {
                all_ok = false;
                break;
            }
            reservedLinks.push_back(lid);
            if (lk.GetBandwidth() < bottleneck_bw) bottleneck_bw = lk.GetBandwidth();
        }

        if (all_ok) {
            // Commit reservation on all links
            for (LINKID lid : reservedLinks) {
                CLink &lk = links[lid];
                lk.CommitReservation(demandid);
                lk.wait_or_not = false;
            }
            // allocate for this node's demand
            relayDemands[demandid] = required;
            // compute execution time as required / bottleneck bandwidth
            if (bottleneck_bw > 0 && bottleneck_bw < std::numeric_limits<RATE>::max()) {
                TIME t = static_cast<TIME>(required) / static_cast<TIME>(bottleneck_bw);
                if (t < executeTime) executeTime = t;
            }
        } else {
            // release any partial reservations we acquired during this attempt
            for (LINKID lid : reservedLinks) {
                links[lid].ReleaseReservation(demandid);
            }
            // add request to waiting queue of involved links to indicate interest (for later arbitration)
            for (LINKID lid : remainingLinks) {
                links[lid].AddToWaitingQueue(demandid, required, demands[demandid].GetArriveTime(), nodeId);
            }
        }
    }

    // If no reservation succeeded, compute minimal wait time until some demand can be served
    if (executeTime == INF) {
        // estimate wait time as min over pending demands of (required / min key rate along path)
        for (DEMANDID demandid : pending) {
            VOLUME required = nodes[nodeId].m_mRelayVolume[demandid];
            if (required == 0) continue;
            NODEID cur = nodeId;
            bool path_ok = true;
            RATE min_qkd_rate = std::numeric_limits<RATE>::max();
            while (cur != demands[demandid].GetSinkId()) {
                auto &nextMap = demands[demandid].m_Path.m_mNextNode;
                if (!nextMap.count(cur)) { path_ok = false; break; }
                NODEID nxt = nextMap[cur];
                auto it = net->m_mNodePairToLink.find(std::make_pair(cur, nxt));
                if (it == net->m_mNodePairToLink.end()) { path_ok = false; break; }
                LINKID lid = it->second;
                CLink &lk = links[lid];
                RATE rate = lk.GetQKDRate();
                if (rate < min_qkd_rate) min_qkd_rate = rate;
                cur = nxt;
                if (min_qkd_rate == 0) { path_ok = false; break; }
            }
            if (!path_ok || min_qkd_rate<=0) continue;
            TIME wait = static_cast<TIME>(required) / static_cast<TIME>(min_qkd_rate);
            if (wait < executeTime) executeTime = wait;
        }
    }

    return executeTime;
}