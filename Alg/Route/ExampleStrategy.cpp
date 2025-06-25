#include "ExampleStrategy.h"
#include "Alg/Network.h"

using namespace route;
bool ExampleStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // BFS 算法实现
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
        // 取出队首元素作为当前节点
        NODEID curNode = toVisit.front();
        toVisit.pop();

        // 如果当前节点是目标节点，则跳出循环
        if (curNode == sinkId)
        {
            break;
        }

        // 遍历当前节点的所有邻接节点
        for (auto adjNodeIter = net->m_vAllNodes[curNode].m_lAdjNodes.begin();
                adjNodeIter != net->m_vAllNodes[curNode].m_lAdjNodes.end();
                adjNodeIter++)
        {
            NODEID adjNode = *adjNodeIter;  // 获取邻接节点 ID

            // 获取边的 ID
            LINKID linkId = net->m_mNodePairToLink[make_pair(curNode, adjNode)];

            // 检查边的权重是否为无穷大
            if (net->m_vAllLinks[linkId].GetWeight() == INF)
            {
                continue; // 跳过权重为无穷大的边
            }

            // 如果邻接节点未被访问过
            if (!visited[adjNode])
            {
                // 标记邻接节点为已访问
                visited[adjNode] = true;

                // 更新邻接节点的前驱节点为当前节点
                preNode[adjNode] = curNode;

                // 将邻接节点加入待访问队列
                toVisit.push(adjNode);

                // 如果邻接节点是目标节点，则跳出内层循环
                if (adjNode == sinkId)
                {
                    break;
                }
            }
        }
    }

    // 如果目标节点没有被访问过，说明不存在从 sourceId 到 sinkId 的路径
    if (!visited[sinkId])
    {
        return false;
    }

    // 从目标节点开始回溯路径，直到源节点
    NODEID curNode = sinkId;
    while (curNode != sourceId)
    {
        // 将当前节点加入路径列表（头部）
        nodeList.push_front(curNode);

        // 获取当前节点的前驱节点
        NODEID pre = preNode[curNode];

        // 获取前驱节点到当前节点的边 ID，并加入边列表（头部）
        LINKID midLink = net->m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);

        // 移动到前驱节点
        curNode = pre;
    }

    // 将源节点加入路径列表（头部）
    nodeList.push_front(sourceId);

    // 返回 true 表示找到了路径
    return true;
}