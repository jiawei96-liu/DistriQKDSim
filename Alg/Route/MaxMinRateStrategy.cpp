#include "MaxMinRateStrategy.h"
#include "Alg/Network.h"
#include <vector>
#include <queue>
#include <limits>

using namespace route;

bool MaxMinRateStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    UINT NodeNum = static_cast<UINT>(net->m_vAllNodes.size());
    
    // 记录到每个节点的最大瓶颈速率
    vector<RATE> maxMinRate(NodeNum, 0.0);
    
    // 记录路径
    vector<NODEID> preNode(NodeNum, -1);
    
    // 记录节点是否已访问
    vector<bool> visited(NodeNum, false);

    // 优先队列，存储 <速率, 节点ID>，按速率降序排列（最大堆）
    using PQNode = pair<RATE, NODEID>;
    priority_queue<PQNode> pq;

    // 初始化源节点
    maxMinRate[sourceId] = std::numeric_limits<RATE>::infinity();
    pq.push({maxMinRate[sourceId], sourceId});
    preNode[sourceId] = sourceId;

    while (!pq.empty()) {
        NODEID curNode = pq.top().second;
        pq.pop();

        if (visited[curNode]) {
            continue;
        }
        visited[curNode] = true;

        if (curNode == sinkId) {
            break;
        }

        for (auto adjNode : net->m_vAllNodes[curNode].m_lAdjNodes) {
            if (!visited[adjNode]) {
                LINKID linkId = net->m_mNodePairToLink[make_pair(curNode, adjNode)];
                RATE linkRate = net->m_vAllLinks[linkId].GetQKDRate();
                
                // 计算通过当前节点到达邻接节点的新瓶颈速率
                RATE newRate = std::min(maxMinRate[curNode], linkRate);

                // 如果找到一条更“宽”的路径，则更新
                if (newRate > maxMinRate[adjNode]) {
                    maxMinRate[adjNode] = newRate;
                    preNode[adjNode] = curNode;
                    pq.push({newRate, adjNode});
                }
            }
        }
    }

    // 如果未到达目标节点，则路径不存在
    if (preNode[sinkId] == -1) {
        return false;
    }

    // 从目标节点回溯以重建路径
    NODEID curNode = sinkId;
    while (curNode != sourceId) {
        nodeList.push_front(curNode);
        NODEID pre = preNode[curNode];
        LINKID midLink = net->m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);
        curNode = pre;
    }
    nodeList.push_front(sourceId);

    return true;
}
