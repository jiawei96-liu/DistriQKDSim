#include "ReservationStrategy.h"
#include "Alg/Network.h"

using namespace sched;

// 端到端预约调度：对于节点上的每个待处理需求，按到达时间优先，尝试在该需求的剩余路径上
// 执行两阶段预约（按链路ID升序调用每条链路的 TryReserve）。
// 只有当路径上所有跳都能成功 TryReserve 时，才对所有跳调用 CommitReservation（并将该需求
// 加入本次 relayDemands 以进行一次性端到端中继）。如果任意一跳失败，则释放已获得的临时预约并将
// 该请求加入各跳的等待队列，等待后续重试。
TIME ReservationStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
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
        if (demands[d].GetArriveTime() > now + SMALLNUM) continue;
        pending.push_back(d);
    }
    if (pending.empty()) return INF;

    std::sort(pending.begin(), pending.end(), [&](DEMANDID a, DEMANDID b){
        return demands[a].GetArriveTime() < demands[b].GetArriveTime();
    });

    for (DEMANDID demandid : pending) {
        VOLUME required = nodes[nodeId].m_mRelayVolume[demandid];
        if (required == 0) continue;

        // build remaining path
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

        // Try two-phase reservation
        std::vector<LINKID> lockOrder = remainingLinks;
        std::sort(lockOrder.begin(), lockOrder.end());
        std::vector<LINKID> reservedLinks;
        bool all_ok = true;
        RATE bottleneck_bw = std::numeric_limits<RATE>::max();

        for (LINKID lid : lockOrder) {
            CLink &lk = links[lid];
            if (lk.GetFaultTime() > 0 && lk.GetFaultTime() <= now) { all_ok = false; break; }
            if (!lk.TryReserve(demandid, required, demands[demandid].GetArriveTime())) { all_ok = false; break; }
            reservedLinks.push_back(lid);
            if (lk.GetBandwidth() < bottleneck_bw) bottleneck_bw = lk.GetBandwidth();
        }

        if (all_ok) {
            for (LINKID lid : reservedLinks) {
                CLink &lk = links[lid];
                lk.CommitReservation(demandid);
                lk.wait_or_not = false;
            }
            relayDemands[demandid] = required;
            if (bottleneck_bw > 0 && bottleneck_bw < std::numeric_limits<RATE>::max()) {
                TIME t = static_cast<TIME>(required) / static_cast<TIME>(bottleneck_bw);
                if (t < executeTime) executeTime = t;
            }
        } else {
            for (LINKID lid : reservedLinks) {
                links[lid].ReleaseReservation(demandid);
            }
            for (LINKID lid : remainingLinks) {
                links[lid].AddToWaitingQueue(demandid, required, demands[demandid].GetArriveTime(), nodeId);
            }
        }
    }

    // if no reservation succeeded, estimate wait time using qkd rates along remaining paths
    if (executeTime == INF) {
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
