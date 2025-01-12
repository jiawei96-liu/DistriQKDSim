#include "KeyRateStrategy.h"
#include "Alg/Network.h"

using namespace route;
// 用二叉堆优化的只考虑keyrate的基于Dijkstra的最短路算法
//TODO：斐波那契堆？
bool KeyRateStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    UINT NodeNum = static_cast<UINT>(net->m_vAllNodes.size());
    vector<NODEID> preNode(NodeNum, sourceId);   // 记录每个节点在最短路径中的前驱节点
    vector<RATE> curDist(NodeNum, INF);         // 从 sourceId 到各节点的当前最短距离
    vector<bool> visited(NodeNum, false);       // 用于记录每个节点是否已被访问
    curDist[sourceId] = 0;
    // 优先队列（小顶堆），存储 pair<距离, 节点ID>
    using PQNode = pair<RATE, NODEID>;
    priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;
    pq.push({0, sourceId}); // 初始节点
    while (!pq.empty())
    {
        auto [curDistVal, curNode] = pq.top();
        pq.pop();
        // 如果当前节点已经访问过，跳过
        if (visited[curNode])
        {
            continue;
        }
        visited[curNode] = true;
        // 如果到达目标节点，停止搜索
        if (curNode == sinkId)
        {
            break;
        }
        // 遍历邻接节点
        for (auto adjNodeIter = net->m_vAllNodes[curNode].m_lAdjNodes.begin();
             adjNodeIter != net->m_vAllNodes[curNode].m_lAdjNodes.end();
             adjNodeIter++)
        {
            NODEID adjNode = *adjNodeIter;
            if (visited[adjNode])
            {
                continue;
            }
            LINKID midLink = net->m_mNodePairToLink[make_pair(curNode, adjNode)];
            RATE newDist = curDist[curNode] + net->m_vAllLinks[midLink].GetQKDRate();
            // Relax 更新距离
            if (newDist < curDist[adjNode])
            {
                curDist[adjNode] = newDist;
                preNode[adjNode] = curNode;
                pq.push({newDist, adjNode});
            }
        }
    }
    // 检查是否成功找到路径
    if (!visited[sinkId])
    {
        return false;
    }
    // 还原路径
    NODEID curNode = sinkId;
    while (curNode != sourceId)
    {
        nodeList.push_front(curNode);
        NODEID pre = preNode[curNode];
        LINKID midLink = net->m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);
        curNode = pre;
    }
    nodeList.push_front(sourceId);
    return true;
}

