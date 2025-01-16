#include "BidBfsStrategy.h"
#include "Alg/Network.h"

using namespace route;

bool BidBfsStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // 检查特殊情况：源节点和目标节点相同
    if (sourceId == sinkId) {
        nodeList.push_back(sourceId);
        return true;
    }

    // 获取网络中节点的总数
    UINT NodeNum = static_cast<UINT>(net->m_vAllNodes.size());

    // 前驱节点记录，用于回溯路径
    vector<NODEID> preNodeSource(NodeNum, -1); // 从源节点出发
    vector<NODEID> preNodeSink(NodeNum, -1);   // 从目标节点出发

    // 访问标记
    vector<bool> visitedSource(NodeNum, false); // 源节点方向的访问标记
    vector<bool> visitedSink(NodeNum, false);   // 目标节点方向的访问标记

    // 队列用于双向 BFS
    queue<NODEID> queueSource; // 从源节点出发的队列
    queue<NODEID> queueSink;   // 从目标节点出发的队列

    // 初始化 BFS
    queueSource.push(sourceId);
    visitedSource[sourceId] = true;

    queueSink.push(sinkId);
    visitedSink[sinkId] = true;

    // 中间节点，表示两个搜索前沿的交汇点
    NODEID meetingNode = -1;

    // 双向 BFS 主循环
    while (!queueSource.empty() && !queueSink.empty()) {
        // 从源方向扩展一步
        if (ExpandFrontier(queueSource, visitedSource, visitedSink, preNodeSource, meetingNode, true)) {
            break;
        }

        // 从目标方向扩展一步
        if (ExpandFrontier(queueSink, visitedSink, visitedSource, preNodeSink, meetingNode, false)) {
            break;
        }
    }

    // 如果没有交汇点，说明没有路径
    if (meetingNode == -1) {
        return false;
    }

    // 回溯路径
    BuildPath(sourceId, sinkId, meetingNode, preNodeSource, preNodeSink, nodeList, linkList);

    return true;
}

bool BidBfsStrategy::ExpandFrontier(queue<NODEID>& frontierQueue, vector<bool>& visitedThisSide,
                                 vector<bool>& visitedOtherSide, vector<NODEID>& preNode,
                                 NODEID& meetingNode, bool isSourceSide) {
    int curSize = frontierQueue.size();

    for (int i = 0; i < curSize; ++i) {
        NODEID curNode = frontierQueue.front();
        frontierQueue.pop();

        // 遍历当前节点的所有邻接节点
        for (auto adjNodeIter = net->m_vAllNodes[curNode].m_lAdjNodes.begin();
             adjNodeIter != net->m_vAllNodes[curNode].m_lAdjNodes.end();
             adjNodeIter++) {
            NODEID adjNode = *adjNodeIter;

            // 获取边的 ID
            LINKID linkId = net->m_mNodePairToLink[make_pair(curNode, adjNode)];

            // 跳过权重为无穷大的边
            if (net->m_vAllLinks[linkId].GetWeight() == INF) {
                continue;
            }

            // 如果邻接节点已被对方访问过，找到交汇点
            if (visitedOtherSide[adjNode]) {
                meetingNode = adjNode;
                preNode[adjNode] = curNode; // 更新前驱节点
                return true;
            }

            // 如果邻接节点未被访问过
            if (!visitedThisSide[adjNode]) {
                visitedThisSide[adjNode] = true;
                preNode[adjNode] = curNode; // 更新前驱节点
                frontierQueue.push(adjNode);
            }
        }
    }

    return false;
}

void BidBfsStrategy::BuildPath(NODEID sourceId, NODEID sinkId, NODEID meetingNode,
                            vector<NODEID>& preNodeSource, vector<NODEID>& preNodeSink,
                            std::list<NODEID>& nodeList, std::list<LINKID>& linkList) {
    // 从 meetingNode 回溯源方向路径
    NODEID curNode = meetingNode;
    while (curNode != sourceId) {
        nodeList.push_front(curNode);
        NODEID pre = preNodeSource[curNode];
        LINKID midLink = net->m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);
        curNode = pre;
    }
    nodeList.push_front(sourceId);

    // 从 meetingNode 回溯目标方向路径
    curNode = meetingNode;
    while (curNode != sinkId) {
        NODEID next = preNodeSink[curNode];
        LINKID midLink = net->m_mNodePairToLink[make_pair(curNode, next)];
        linkList.push_back(midLink);
        curNode = next;
        nodeList.push_back(curNode);
    }
}
