#include "BgpStrategy.h"
#include "Alg/Network.h"
#include <limits>
#include <queue>
#include <vector>
#include <functional>
#include <unordered_set>
#include <map>
#include <utility>

using namespace route;

// BGP 路由算法骨架：仅实现主干结构，实际可根据子域/网关等属性扩展
bool BgpStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // 简单实现：优先跨域，模拟BGP决策
    // 1. 标记所有网关节点
    // 2. 通过网关跨域，优先选择跨域链路
    // 3. 终点在目标子域内用最短路
    // 这里只做结构演示，具体可根据实际需求完善
    nodeList.clear();
    linkList.clear();
    if (sourceId == sinkId) {
        nodeList.push_back(sourceId);
        return true;
    }
    int srcDomain = net->m_vAllNodes[sourceId].GetSubdomainId();
    int dstDomain = net->m_vAllNodes[sinkId].GetSubdomainId();
    // 1. 同域内直接最短路
    if (srcDomain == dstDomain) {
        // BFS最短路
        std::map<NODEID, NODEID> prev;
        std::queue<NODEID> q;
        std::unordered_set<NODEID> visited;
        q.push(sourceId);
        visited.insert(sourceId);
        bool found = false;
        while (!q.empty() && !found) {
            NODEID u = q.front(); q.pop();
            for (NODEID v : net->m_vAllNodes[u].m_lAdjNodes) {
                if (visited.count(v)) continue;
                if (net->m_vAllNodes[v].GetSubdomainId() != srcDomain) continue; // 只在本域内
                prev[v] = u;
                if (v == sinkId) { found = true; break; }
                q.push(v);
                visited.insert(v);
            }
        }
        if (!found) return false;
        NODEID cur = sinkId;
        while (cur != sourceId) {
            nodeList.push_front(cur);
            cur = prev[cur];
        }
        nodeList.push_front(sourceId);
    } else {
        // 2. 跨域：源域->网关->目标域->目的
        // 2.1 找到源域所有网关
        std::vector<NODEID> srcGateways;
        for (const auto& node : net->m_vAllNodes) {
            if (node.GetSubdomainId() == srcDomain && node.IsGateway())
                srcGateways.push_back(node.GetNodeId());
        }
        // 2.2 找到目标域所有网关
        std::vector<NODEID> dstGateways;
        for (const auto& node : net->m_vAllNodes) {
            if (node.GetSubdomainId() == dstDomain && node.IsGateway())
                dstGateways.push_back(node.GetNodeId());
        }
        // 2.3 源到所有本域网关的最短路
        std::map<NODEID, std::vector<NODEID>> srcPaths;
        std::map<NODEID, int> srcDist;
        for (NODEID gw : srcGateways) {
            std::queue<NODEID> q;
            std::unordered_set<NODEID> visited;
            std::map<NODEID, NODEID> prev;
            q.push(sourceId);
            visited.insert(sourceId);
            bool found = false;
            while (!q.empty() && !found) {
                NODEID u = q.front(); q.pop();
                for (NODEID v : net->m_vAllNodes[u].m_lAdjNodes) {
                    if (visited.count(v)) continue;
                    if (net->m_vAllNodes[v].GetSubdomainId() != srcDomain) continue;
                    prev[v] = u;
                    if (v == gw) { found = true; break; }
                    q.push(v);
                    visited.insert(v);
                }
            }
            if (found) {
                std::vector<NODEID> path;
                NODEID cur = gw;
                while (cur != sourceId) {
                    path.push_back(cur);
                    cur = prev[cur];
                }
                path.push_back(sourceId);
                std::reverse(path.begin(), path.end());
                srcPaths[gw] = path;
                srcDist[gw] = path.size();
            }
        }
        // 2.4 目标域网关到目的节点的最短路
        std::map<NODEID, std::vector<NODEID>> dstPaths;
        std::map<NODEID, int> dstDist;
        for (NODEID gw : dstGateways) {
            std::queue<NODEID> q;
            std::unordered_set<NODEID> visited;
            std::map<NODEID, NODEID> prev;
            q.push(gw);
            visited.insert(gw);
            bool found = false;
            while (!q.empty() && !found) {
                NODEID u = q.front(); q.pop();
                for (NODEID v : net->m_vAllNodes[u].m_lAdjNodes) {
                    if (visited.count(v)) continue;
                    if (net->m_vAllNodes[v].GetSubdomainId() != dstDomain && v != sinkId) continue;
                    prev[v] = u;
                    if (v == sinkId) { found = true; break; }
                    q.push(v);
                    visited.insert(v);
                }
            }
            if (found) {
                std::vector<NODEID> path;
                NODEID cur = sinkId;
                while (cur != gw) {
                    path.push_back(cur);
                    cur = prev[cur];
                }
                path.push_back(gw);
                std::reverse(path.begin(), path.end());
                dstPaths[gw] = path;
                dstDist[gw] = path.size();
            }
        }
        // 2.5 查找所有跨域网关对
        int minTotal = std::numeric_limits<int>::max();
        std::vector<NODEID> bestSrcPath, bestDstPath;
        NODEID bestSrcGw = -1, bestDstGw = -1;
        for (NODEID srcGw : srcGateways) {
            for (NODEID dstGw : dstGateways) {
                // 检查是否有直接跨域链路
                if (net->m_mNodePairToLink.count({srcGw, dstGw}) == 0) continue;
                if (srcPaths.count(srcGw) == 0 || dstPaths.count(dstGw) == 0) continue;
                int total = srcDist[srcGw] + 1 + dstDist[dstGw];
                if (total < minTotal) {
                    minTotal = total;
                    bestSrcPath = srcPaths[srcGw];
                    bestDstPath = dstPaths[dstGw];
                    bestSrcGw = srcGw;
                    bestDstGw = dstGw;
                }
            }
        }
        if (minTotal == std::numeric_limits<int>::max()) return false;
        // 拼接完整路径
        for (NODEID n : bestSrcPath) nodeList.push_back(n);
        if (bestSrcGw != bestDstGw) nodeList.push_back(bestDstGw); // 跨域
        for (size_t i = 1; i < bestDstPath.size(); ++i) nodeList.push_back(bestDstPath[i]);
    }
    // 生成 linkList
    auto it = nodeList.begin();
    auto next = it; ++next;
    while (next != nodeList.end()) {
        LINKID lid = net->m_mNodePairToLink.at({*it, *next});
        linkList.push_back(lid);
        ++it; ++next;
    }
    return true;
}
