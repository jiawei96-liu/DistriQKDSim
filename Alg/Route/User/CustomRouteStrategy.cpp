#include "Alg/Route/CustomRouteStrategy.h"
#include "Alg/Network.h"

using namespace route;
extern "C" route::CustomRouteStrategy* createRouteStrategy(CNetwork* net) {
    return new route::CustomRouteStrategy(net);
}
bool CustomRouteStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // 自定义路由算法实现
    // 获取网络中节点的总数
    UINT NodeNum = static_cast<UINT>(net->m_vAllNodes.size());

    // 用于记录每个节点在最短路径中的前驱节点，初始化为 -1
    vector<NODEID> preNode(NodeNum, -1);

    // 用于记录每个节点是否已被访问，初始化为 false
    vector<bool> visited(NodeNum, false);

    // 队列用于进行广度优先搜索（BFS）
    queue<NODEID> toVisit;

    // 将源节点加入待访问队列并标记为已访问
    toVisit.push(sourceId);
    visited[sourceId] = true;

    // 开始进行 BFS 搜索
    while (!toVisit.empty())
    {
        NODEID curNode = toVisit.front();
        toVisit.pop();

        if (curNode == sinkId) { break; }

        for (auto it = net->m_vAllNodes[curNode].m_lAdjNodes.begin();
             it != net->m_vAllNodes[curNode].m_lAdjNodes.end(); ++it)
        {
            NODEID adjNode = *it;
            LINKID linkId = net->m_mNodePairToLink[make_pair(curNode, adjNode)];

            if (net->m_vAllLinks[linkId].GetWeight() == INF) { continue; }

            if (!visited[adjNode]) {
                visited[adjNode] = true;
                preNode[adjNode] = curNode;
                toVisit.push(adjNode);
                if (adjNode == sinkId) { break; }
            }
        }
    }

    if (!visited[sinkId]) { return false; }

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