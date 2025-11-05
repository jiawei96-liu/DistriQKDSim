#include "MinTimeStrategy.h"
#include "Alg/Network.h"
#include <queue>
#include <limits>

using namespace route;

// 计算单个链路的传输时间
double MinTimeStrategy::CalculateLinkTime(LINKID linkId, double dataAmount) {
    // 获取链路对象
    const auto& link = net->m_vAllLinks[linkId];
    
    // 获取当前可用的密钥数量
    double availableKeys = link.m_KeyManager.m_dAvailableKeys;
    
    // 获取密钥生成速率
    double keyRate = link.m_KeyManager.m_dKeyRate;
    
    // 计算在该链路上的传输时间
    // 如果已有密钥足够,传输时间为0
    // 否则需要等待生成足够的密钥: (数据量 - 已有密钥量) / 密钥生成速率
    if (availableKeys >= dataAmount) {
        // 密钥足够,传输时间为0
        return 0.0;
    } else if (keyRate > 0) {
        // 密钥不足,需要等待生成
        return (dataAmount - availableKeys) / keyRate;
    } else {
        // 如果密钥生成速率为0,无法传输,返回无穷大
        return std::numeric_limits<double>::infinity();
    }
}

bool MinTimeStrategy::Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList, double dataAmount) {
    // 使用Dijkstra算法寻找传输时间最短的路径
    
    // 获取网络中节点的总数
    UINT NodeNum = static_cast<UINT>(net->m_vAllNodes.size());

    // 用于记录每个节点的最短传输时间,初始化为无穷大
    vector<double> minTime(NodeNum, std::numeric_limits<double>::infinity());

    // 用于记录每个节点在最短路径中的前驱节点,初始化为 -1
    vector<NODEID> preNode(NodeNum, -1);

    // 用于记录每个节点在最短路径中的前驱链路,初始化为 -1
    vector<LINKID> preLink(NodeNum, -1);

    // 用于记录每个节点是否已被访问(已确定最短时间),初始化为 false
    vector<bool> visited(NodeNum, false);

    // 优先队列,存储 (传输时间, 节点ID) 对,按传输时间从小到大排序
    priority_queue<pair<double, NODEID>, vector<pair<double, NODEID>>, greater<pair<double, NODEID>>> pq;

    // 初始化源节点
    minTime[sourceId] = 0.0;
    pq.push(make_pair(0.0, sourceId));

    // 开始Dijkstra算法
    while (!pq.empty())
    {
        // 取出当前传输时间最小的节点
        pair<double, NODEID> top = pq.top();
        pq.pop();
        
        double currentTime = top.first;
        NODEID curNode = top.second;

        // 如果当前节点已被访问过,跳过
        if (visited[curNode])
        {
            continue;
        }

        // 标记当前节点为已访问
        visited[curNode] = true;

        // 如果当前节点是目标节点,可以提前结束
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

            // 如果邻接节点已被访问过,跳过
            if (visited[adjNode])
            {
                continue;
            }

            // 获取边的 ID
            LINKID linkId = net->m_mNodePairToLink[make_pair(curNode, adjNode)];

            // 检查边的权重是否为无穷大
            if (net->m_vAllLinks[linkId].GetWeight() == INF)
            {
                continue; // 跳过权重为无穷大的边
            }

            // 计算该链路的传输时间
            double edgeTime = CalculateLinkTime(linkId, dataAmount);
            
            // 如果链路无法传输(返回无穷大),跳过
            if (edgeTime == std::numeric_limits<double>::infinity()) {
                continue;
            }
            
            // Dijkstra松弛操作: 新时间 = 到当前节点的时间 + 当前边的时间
            double newTime = minTime[curNode] + edgeTime;

            // 如果找到更短的传输时间,更新
            if (newTime < minTime[adjNode])
            {
                minTime[adjNode] = newTime;
                preNode[adjNode] = curNode;
                preLink[adjNode] = linkId;
                pq.push(make_pair(newTime, adjNode));
            }
        }
    }

    // 如果目标节点的最短时间仍为无穷大,说明不存在从 sourceId 到 sinkId 的路径
    if (minTime[sinkId] == std::numeric_limits<double>::infinity())
    {
        return false;
    }

    // 从目标节点开始回溯路径,直到源节点
    NODEID curNode = sinkId;
    while (curNode != sourceId)
    {
        // 将当前节点加入路径列表(头部)
        nodeList.push_front(curNode);

        // 将前驱链路加入链路列表(头部)
        linkList.push_front(preLink[curNode]);

        // 移动到前驱节点
        curNode = preNode[curNode];
    }

    // 将源节点加入路径列表(头部)
    nodeList.push_front(sourceId);

    // 返回 true 表示找到了路径
    return true;
}
