#include "OspfStrategy.h"
#include "Alg/Network.h"
#include <limits>
#include <queue>
#include <vector>
#include <functional>
#include <unordered_set>
#include <map>
#include <utility>

using namespace route;

bool OspfStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    
    // OSPF模拟：动态发现网络拓扑
    
    // 存储动态发现的拓扑信息：邻居节点及其链路权重
    std::map<NODEID, std::vector<std::pair<NODEID, WEIGHT>>> localTopology;
    std::unordered_set<NODEID> discoveredNodes;
    std::queue<NODEID> discoveryQueue;

    discoveryQueue.push(sourceId);
    discoveredNodes.insert(sourceId);

    bool sinkFound = false;
    
    // 动态邻居发现和LSA泛洪过程
    while (!discoveryQueue.empty() && !sinkFound) {
        NODEID curNode = discoveryQueue.front();
        discoveryQueue.pop();

        localTopology[curNode] = {}; // 初始化当前节点的邻接列表

        // 遍历当前节点的邻居，模拟LSA生成和泛洪
        for (NODEID adjNode : net->m_vAllNodes[curNode].m_lAdjNodes) {
            LINKID linkId = net->m_mNodePairToLink.at({curNode, adjNode});
            WEIGHT linkWeight = net->m_vAllLinks[linkId].GetWeight();

            if (linkWeight == INF) {
                continue;
            }

            localTopology[curNode].push_back({adjNode, linkWeight});

            if (discoveredNodes.find(adjNode) == discoveredNodes.end()) {
                discoveredNodes.insert(adjNode);
                discoveryQueue.push(adjNode);
                if (adjNode == sinkId) {
                    sinkFound = true;
                }
            }
        }
    }

    // 如果目标节点未被发现，则路径不可达
    if (!sinkFound) {
        return false;
    }

    // 基于已发现的本地拓扑（LSDB）运行Dijkstra算法
    
    // 存储距离和前驱节点
    std::map<NODEID, WEIGHT> distance;
    std::map<NODEID, NODEID> preNode;

    for (NODEID node : discoveredNodes) {
        distance[node] = std::numeric_limits<WEIGHT>::max();
        preNode[node] = -1;
    }

    distance[sourceId] = 0;
    
    // 优先队列，用于Dijkstra算法
    std::priority_queue<std::pair<WEIGHT, NODEID>,
                        std::vector<std::pair<WEIGHT, NODEID>>,
                        std::greater<std::pair<WEIGHT, NODEID>>> pq;
    
    pq.push({0, sourceId});

    while (!pq.empty()) {
        WEIGHT curDist = pq.top().first;
        NODEID curNode = pq.top().second;
        pq.pop();

        if (curDist > distance[curNode]) {
            continue;
        }

        if (curNode == sinkId) {
            break;
        }

        // 遍历本地拓扑中的邻居
        if (localTopology.count(curNode)) {
            for (const auto& neighbor : localTopology.at(curNode)) {
                NODEID adjNode = neighbor.first;
                WEIGHT linkWeight = neighbor.second;
                
                WEIGHT newDist = curDist + linkWeight;
                
                if (newDist < distance[adjNode]) {
                    distance[adjNode] = newDist;
                    preNode[adjNode] = curNode;
                    pq.push({newDist, adjNode});
                }
            }
        }
    }
    
    // 如果目标节点在Dijkstra后仍不可达，则返回false
    if (distance[sinkId] == std::numeric_limits<WEIGHT>::max()) {
        return false;
    }

    // 回溯路径
    NODEID curNode = sinkId;
    while (curNode != sourceId) {
        nodeList.push_front(curNode);
        NODEID pre = preNode.at(curNode);
        LINKID midLink = net->m_mNodePairToLink.at({pre, curNode});
        linkList.push_front(midLink);
        curNode = pre;
    }
    nodeList.push_front(sourceId);

    return true;
}