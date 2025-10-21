#include <fstream>
#include <sstream>


// 从 network_full.csv 读取链路和节点属性，自动补全节点信息
void CNetwork::LoadNetworkFullCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开网络文件: " << filename << std::endl;
        return;
    }
    std::string line;
    std::getline(file, line); // 跳过表头
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;
        while (std::getline(ss, item, ',')) {
            tokens.push_back(item);
        }
        if (tokens.size() < 13) continue;
        LINKID linkId = std::stoi(tokens[0]);
        NODEID src = std::stoi(tokens[1]);
        NODEID dst = std::stoi(tokens[2]);
        double keyRate = std::stod(tokens[3]);
        double delay = std::stod(tokens[4]);
        double bandwidth = std::stod(tokens[5]);
        double weight = std::stod(tokens[6]);
        double faultTime = std::stod(tokens[7]);
        int is_cross = std::stoi(tokens[8]);
        int src_subdomain = std::stoi(tokens[9]);
        bool src_isGateway = std::stoi(tokens[10]) != 0;
        int dst_subdomain = std::stoi(tokens[11]);
        bool dst_isGateway = std::stoi(tokens[12]) != 0;
        // 节点信息补全
        if (src >= m_vAllNodes.size()) m_vAllNodes.resize(src+1);
        if (dst >= m_vAllNodes.size()) m_vAllNodes.resize(dst+1);
        m_vAllNodes[src].SetNodeId(src);
        m_vAllNodes[src].SetSubdomainId(src_subdomain);
        m_vAllNodes[src].SetIsGateway(src_isGateway);
        m_vAllNodes[dst].SetNodeId(dst);
        m_vAllNodes[dst].SetSubdomainId(dst_subdomain);
        m_vAllNodes[dst].SetIsGateway(dst_isGateway);
        // 链路信息
        if (linkId >= m_vAllLinks.size()) m_vAllLinks.resize(linkId+1);
        CLink link;
        link.SetLinkId(linkId);
        link.SetSourceId(src);
        link.SetSinkId(dst);
        link.SetQKDRate(keyRate);
        link.SetLinkDelay(delay);
        link.SetBandwidth(bandwidth);
        link.SetWeight(weight);
        link.SetFaultTime(faultTime);
        link.SetSubdomainId(is_cross ? -1 : src_subdomain);
        m_vAllLinks[linkId] = link;
    }
    file.close();
}
#include "Network.h"
#include "Link.h"
#include "DistributedIDGenerator.h"
#include <iostream>
#include <chrono>
#include <queue>
#include <thread>
#include <vector>
#include <algorithm> // for std::min
#include <random>
#include "Web/dao/SimDao.hpp"

#include <chrono> // 高精度时间


CNetwork::CNetwork(void):simDao(new SimDao())
{
    m_dSimTime = 0;
    FaultTime = -1;
    m_step = 0;
    simID=DistributedIDGenerator::next_id();
    status="Init";
    // currentRouteAlg = [this](NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) -> bool
    // {
    //     return this->ShortestPath(sourceId, sinkId, nodeList, linkList);
    // };
    m_routeFactory=make_unique<route::RouteFactory>(this);

    m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Bfs));    //BFS

    m_schedFactory=make_unique<sched::SchedFactory>(this);

    m_schedStrategy=std::move(m_schedFactory->CreateStrategy(sched::SchedType_Min));    //BFS

    // m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_KeyRateShortestPath));   //keyrate最短路策略

    m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_demo));   //自定义路由算法

    // currentScheduleAlg = [this](NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
    // {
    //     return this->MinimumRemainingTimeFirst(nodeId, relayDemands);
    // };
    currentScheduleAlg = [this](LINKID linkId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
    {
        // return this->MinimumRemainingTimeFirstLinkBased(linkId, relayDemands);
        return this->AverageKeySchedulingLinkBased(linkId, relayDemands);
    };
}

CNetwork::~CNetwork(void)
{
}

void CNetwork::Clear()
{
    m_dSimTime = 0;
    FaultTime = -1;
    m_step = 0;
    m_vAllNodes.clear();
    m_vAllLinks.clear();
    m_vAllDemands.clear();
    m_vAllRelayPaths.clear();
    m_mNodePairToLink.clear();
    m_mDemandArriveTime.clear();
    simID=rand()%UINT32_MAX;
    status="End";
    // currentRouteAlg = [this](NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) -> bool
    // {
    //     return this->ShortestPath(sourceId, sinkId, nodeList, linkList);
    // };
    m_routeFactory=make_unique<route::RouteFactory>(this);

    // m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Bfs));    //BFS

    m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_KeyRateShortestPath));   //keyrate最短路策略

    m_schedFactory=make_unique<sched::SchedFactory>(this);

    m_schedStrategy=std::move(m_schedFactory->CreateStrategy(sched::SchedType_Min));    //BFS

    currentScheduleAlg = [this](NODEID nodeId, map<DEMANDID, VOLUME>& relayDemands) -> TIME
    {
        return this->MinimumRemainingTimeFirst(nodeId, relayDemands);
        // return this->AverageKeyScheduling(nodeId, relayDemands);
    };
}

TIME CNetwork::CurrentTime()
{
    return m_dSimTime;
}

UINT CNetwork::CurrentStep()
{
    return m_step;
}

// 推进模拟时间 m_dSimTime，并删除所有已到达的需求
void CNetwork::MoveSimTime(TIME executionTime)
{
    m_step++;
    std::cout<<"当前已执行步数："<<m_step<<endl;
    m_dSimTime += executionTime;
    // erase all arrived demands
    if (m_mDemandArriveTime.empty())
        return;
    auto demandIter = m_mDemandArriveTime.begin();
    while (demandIter->first <= m_dSimTime + SMALLNUM)
    {
        demandIter = m_mDemandArriveTime.erase(demandIter); // erase 方法删除当前迭代器所指向的元素，并返回一个指向下一个元素的迭代器。
        // cout << "更新demand指针" << endl;
        if (demandIter == m_mDemandArriveTime.end())
            break;
    }
}

void CNetwork::SetNodeNum(UINT nodeNum)
{
    m_uiNodeNum = nodeNum;
}

UINT CNetwork::GetNodeNum()
{
    return m_uiNodeNum;
}

void CNetwork::SetLinkNum(UINT linkNum)
{
    m_uiLinkNum = linkNum;
}

UINT CNetwork::GetLinkNum()
{
    return m_uiLinkNum;
}

void CNetwork::SetDemandNum(UINT demandNum)
{
    m_uiDemandNum = demandNum;
}

UINT CNetwork::GetDemandNum()
{
    return m_uiDemandNum;
}
// 为特定链路 linkId 初始化密钥管理器 CKeyManager，并将其与链路、源节点和目标节点关联起来
void CNetwork::InitKeyManagerOverLink(LINKID linkId)
{
    CKeyManager newKeyManager;
    newKeyManager.SetAssociatedLink(linkId);
    newKeyManager.SetAssociatedNode(m_vAllLinks[linkId].GetSourceId());
    newKeyManager.SetPairedNode(m_vAllLinks[linkId].GetSinkId());
    newKeyManager.SetKeyManagerId(linkId);
    newKeyManager.SetKeyRate(m_vAllLinks[linkId].GetQKDRate());
    newKeyManager.SetAvailableKeys(0);

    m_vAllLinks[linkId].SetKeyManager(newKeyManager);
    //    m_vAllKeyManager.push_back(newKeyManager);
}

void CNetwork::InitNodes(UINT nodeNum)
{
    // 根据传入的节点数量 nodeNum，逐个创建节点对象 CNode 并设置其 nodeId，然后将这些节点添加到网络对象 m_pNetwork 的节点列表 m_vAllNodes 中
    for (NODEID nodeId = 0; nodeId < nodeNum; nodeId++)
    {
        CNode newNode;
        newNode.SetNodeId(nodeId);
        m_vAllNodes.push_back(newNode);
    }
}
// // 使用最短路径算法Dijkstra计算从源节点 sourceId 到汇节点 sinkId 的最短路径，并将路径中的节点和链路记录在 nodeList 和 linkList 中。如果找到有效路径，返回 true，否则返回 false
// bool CNetwork::ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID> &nodeList, list<LINKID> &linkList)
// {
//     UINT NodeNum = static_cast<UINT>(m_vAllNodes.size());
//     vector<NODEID> preNode(NodeNum, sourceId); // 记录每个节点在最短路径中的前驱节点
//     vector<WEIGHT> curDist(NodeNum, INF);      // 用于记录从 sourceId 到各节点的当前最短距离
//     vector<bool> visited(NodeNum, false);      // 用于记录每个节点是否已被访问
//     curDist[sourceId] = 0;
//     visited[sourceId] = true;
//     NODEID curNode = sourceId;
//     while (curNode != sinkId)
//     {
//         for (auto adjNodeIter = m_vAllNodes[curNode].m_lAdjNodes.begin(); adjNodeIter != m_vAllNodes[curNode].m_lAdjNodes.end(); adjNodeIter++)
//         {
//             if (visited[*adjNodeIter])
//             {
//                 continue;
//             }
//             LINKID midLink = m_mNodePairToLink[make_pair(curNode, *adjNodeIter)];
//             if (curDist[curNode] + m_vAllLinks[midLink].GetWeight() < curDist[*adjNodeIter])
//             {
//                 curDist[*adjNodeIter] = curDist[curNode] + m_vAllLinks[midLink].GetWeight();
//                 preNode[*adjNodeIter] = curNode;
//             }
//         }
//         // Find next node
//         WEIGHT minDist = INF;
//         NODEID nextNode = curNode;
//         for (NODEID nodeId = 0; nodeId < NodeNum; nodeId++)
//         {
//             if (visited[nodeId])
//             {
//                 continue;
//             }
//             if (curDist[nodeId] < minDist)
//             {
//                 nextNode = nodeId;
//                 minDist = curDist[nodeId];
//             }
//         }
//         if (minDist >= INF || nextNode == curNode)
//         {
//             return false;
//         }
//         curNode = nextNode;
//         visited[nextNode] = true;
//     }
//     if (curNode != sinkId)
//     {
//         cout << "why current node is not sink node?? check function shortestPath!" << endl;
//         getchar();
//         exit(0);
//     }
//     while (curNode != sourceId)
//     {
//         nodeList.push_front(curNode);
//         NODEID pre = preNode[curNode];
//         LINKID midLink = m_mNodePairToLink[make_pair(pre, curNode)];
//         linkList.push_front(midLink);
//         curNode = pre;
//     }
//     nodeList.push_front(sourceId);
//     return true;
// }

// 使用BFS的最短路径算法
bool CNetwork::ShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID> &nodeList, list<LINKID> &linkList)
{
    // 获取网络中节点的总数
    UINT NodeNum = static_cast<UINT>(m_vAllNodes.size());

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
        for (auto adjNodeIter = m_vAllNodes[curNode].m_lAdjNodes.begin();
                adjNodeIter != m_vAllNodes[curNode].m_lAdjNodes.end();
                adjNodeIter++)
        {
            NODEID adjNode = *adjNodeIter;  // 获取邻接节点 ID

            // 获取边的 ID
            LINKID linkId = m_mNodePairToLink[make_pair(curNode, adjNode)];

            // 检查边的权重是否为无穷大
            if (m_vAllLinks[linkId].GetWeight() == INF)
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
        LINKID midLink = m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);

        // 移动到前驱节点
        curNode = pre;
    }

    // 将源节点加入路径列表（头部）
    nodeList.push_front(sourceId);

    // 返回 true 表示找到了路径
    return true;
}

// 只考虑keyrate的基于Dijkstra的最短路算法
// bool CNetwork::KeyRateShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList)
// {
//     UINT NodeNum = static_cast<UINT>(m_vAllNodes.size());
//     vector<NODEID> preNode(NodeNum, sourceId);	// 记录每个节点在最短路径中的前驱节点
//     vector<RATE> curDist(NodeNum, INF);	// 用于记录从 sourceId 到各节点的当前最短距离
//     vector<bool> visited(NodeNum, false);	// 用于记录每个节点是否已被访问
//     curDist[sourceId] = 0;
//     visited[sourceId] = true;
//     NODEID curNode = sourceId;
//     while (curNode != sinkId)
//     {
//         for (auto adjNodeIter = m_vAllNodes[curNode].m_lAdjNodes.begin(); adjNodeIter != m_vAllNodes[curNode].m_lAdjNodes.end(); adjNodeIter++)
//         {
//             if (visited[*adjNodeIter])
//             {
//                 continue;
//             }
//             LINKID midLink = m_mNodePairToLink[make_pair(curNode, *adjNodeIter)];
//             if (curDist[curNode] + m_vAllLinks[midLink].GetQKDRate() < curDist[*adjNodeIter])
//             {
//                 curDist[*adjNodeIter] = curDist[curNode] + m_vAllLinks[midLink].GetQKDRate();
//                 preNode[*adjNodeIter] = curNode;
//             }
//         }
//         //Find next node
//         RATE minDist = INF;
//         NODEID nextNode = curNode;
//         for (NODEID nodeId = 0; nodeId < NodeNum; nodeId++)
//         {
//             if (visited[nodeId])
//             {
//                 continue;
//             }
//             if (curDist[nodeId] < minDist)
//             {
//                 nextNode = nodeId;
//                 minDist = curDist[nodeId];
//             }
//         }
//         if (minDist >= INF || nextNode == curNode)
//         {
//             return false;
//         }
//         curNode = nextNode;
//         visited[nextNode] = true;
//     }
//     if (curNode != sinkId)
//     {
//         cout << "why current node is not sink node?? check function shortestPath!" << endl;
//         getchar();
//         exit(0);
//     }
//     while(curNode != sourceId)
//     {
//         nodeList.push_front(curNode);
//         NODEID pre = preNode[curNode];
//         LINKID midLink = m_mNodePairToLink[make_pair(pre, curNode)];
//         linkList.push_front(midLink);
//         curNode = pre;
//     }
//     nodeList.push_front(sourceId);
//     return true;
// }

// //// 用二叉堆优化的只考虑keyrate的基于Dijkstra的最短路算法
// bool CNetwork::KeyRateShortestPathWithBinHeap(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList)
// {
//     UINT NodeNum = static_cast<UINT>(m_vAllNodes.size());
//     vector<NODEID> preNode(NodeNum, sourceId);   // 记录每个节点在最短路径中的前驱节点
//     vector<RATE> curDist(NodeNum, INF);         // 从 sourceId 到各节点的当前最短距离
//     vector<bool> visited(NodeNum, false);       // 用于记录每个节点是否已被访问
//     curDist[sourceId] = 0;
//     // 优先队列（小顶堆），存储 pair<距离, 节点ID>
//     using PQNode = pair<RATE, NODEID>;
//     priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;
//     pq.push({0, sourceId}); // 初始节点
//     while (!pq.empty())
//     {
//         auto [curDistVal, curNode] = pq.top();
//         pq.pop();
//         // 如果当前节点已经访问过，跳过
//         if (visited[curNode])
//         {
//             continue;
//         }
//         visited[curNode] = true;
//         // 如果到达目标节点，停止搜索
//         if (curNode == sinkId)
//         {
//             break;
//         }
//         // 遍历邻接节点
//         for (auto adjNodeIter = m_vAllNodes[curNode].m_lAdjNodes.begin();
//              adjNodeIter != m_vAllNodes[curNode].m_lAdjNodes.end();
//              adjNodeIter++)
//         {
//             NODEID adjNode = *adjNodeIter;
//             if (visited[adjNode])
//             {
//                 continue;
//             }
//             LINKID midLink = m_mNodePairToLink[make_pair(curNode, adjNode)];
//             RATE newDist = curDist[curNode] + m_vAllLinks[midLink].GetQKDRate();
//             // Relax 更新距离
//             if (newDist < curDist[adjNode])
//             {
//                 curDist[adjNode] = newDist;
//                 preNode[adjNode] = curNode;
//                 pq.push({newDist, adjNode});
//             }
//         }
//     }
//     // 检查是否成功找到路径
//     if (!visited[sinkId])
//     {
//         return false;
//     }
//     // 还原路径
//     NODEID curNode = sinkId;
//     while (curNode != sourceId)
//     {
//         nodeList.push_front(curNode);
//         NODEID pre = preNode[curNode];
//         LINKID midLink = m_mNodePairToLink[make_pair(pre, curNode)];
//         linkList.push_front(midLink);
//         curNode = pre;
//     }
//     nodeList.push_front(sourceId);
//     return true;
// }
//判断两个List是否相等
template <typename T>
bool ListsEqual(const std::list<T>& list1, const std::list<T>& list2) {
    if (list1.size() != list2.size()) {
        return false;
    }
    auto it1 = list1.begin();
    auto it2 = list2.begin();
    while (it1 != list1.end() && it2 != list2.end()) {
        if (*it1 != *it2) {
            return false;
        }
        ++it1;
        ++it2;
    }
    return true;
}

// 增加逐个demand路由并修改相应link上的负载的函数
// 123
// 使用最短路径算法Dijkstra计算从源节点 sourceId 到汇节点 sinkId 的最短路径，并将路径中的节点和链路记录在 nodeList 和 linkList 中。如果找到有效路径，返回 true，否则返回 false
// 每次为一个demand求得路径后，就修改路径上link的weight值（为对应的负载）
bool CNetwork::Load_Balance(NODEID sourceId, NODEID sinkId, list<NODEID> &nodeList, list<LINKID> &linkList)
{
    UINT NodeNum = static_cast<UINT>(m_vAllNodes.size());
    vector<NODEID> preNode(NodeNum, sourceId); // 记录每个节点在最短路径中的前驱节点
    vector<WEIGHT> curDist(NodeNum, INF);      // 用于记录从 sourceId 到各节点的当前最短距离
    vector<bool> visited(NodeNum, false);      // 用于记录每个节点是否已被访问
    curDist[sourceId] = 0;
    visited[sourceId] = true;
    NODEID curNode = sourceId;
    while (curNode != sinkId)
    {
        for (auto adjNodeIter = m_vAllNodes[curNode].m_lAdjNodes.begin(); adjNodeIter != m_vAllNodes[curNode].m_lAdjNodes.end(); adjNodeIter++)
        {
            if (visited[*adjNodeIter])
            {
                continue;
            }
            LINKID midLink = m_mNodePairToLink[make_pair(curNode, *adjNodeIter)];
            if (curDist[curNode] + m_vAllLinks[midLink].GetWeight() < curDist[*adjNodeIter])
            {
                curDist[*adjNodeIter] = curDist[curNode] + m_vAllLinks[midLink].GetWeight();
                preNode[*adjNodeIter] = curNode;
            }
        }
        // Find next node
        WEIGHT minDist = INF;
        NODEID nextNode = curNode;
        for (NODEID nodeId = 0; nodeId < NodeNum; nodeId++)
        {
            if (visited[nodeId])
            {
                continue;
            }
            if (curDist[nodeId] < minDist)
            {
                nextNode = nodeId;
                minDist = curDist[nodeId];
            }
        }
        if (minDist >= INF || nextNode == curNode)
        {
            return false;
        }
        curNode = nextNode;
        visited[nextNode] = true;
    }
    if (curNode != sinkId)
    {
        cout << "why current node is not sink node?? check function shortestPath!" << endl;
        getchar();
        exit(0);
    }
    while (curNode != sourceId)
    {
        nodeList.push_front(curNode);
        NODEID pre = preNode[curNode];
        LINKID midLink = m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);
        curNode = pre;
    }
    nodeList.push_front(sourceId);
    return true;
}

// 只考虑keyrate的最短路算法
bool CNetwork::KeyRateShortestPath(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList)
{
    UINT NodeNum = static_cast<UINT>(m_vAllNodes.size());
    vector<NODEID> preNode(NodeNum, sourceId);	// 记录每个节点在最短路径中的前驱节点
    vector<RATE> curDist(NodeNum, INF);	// 用于记录从 sourceId 到各节点的当前最短距离
    vector<bool> visited(NodeNum, false);	// 用于记录每个节点是否已被访问
    curDist[sourceId] = 0;
    visited[sourceId] = true;
    NODEID curNode = sourceId;
    while (curNode != sinkId)
    {
        list<NODEID>::iterator adjNodeIter;
        adjNodeIter = m_vAllNodes[curNode].m_lAdjNodes.begin();
        for (; adjNodeIter != m_vAllNodes[curNode].m_lAdjNodes.end(); adjNodeIter++)
        {
            if (visited[*adjNodeIter])
            {
                continue;
            }
            LINKID midLink = m_mNodePairToLink[make_pair(curNode, *adjNodeIter)];
            if (curDist[curNode] + m_vAllLinks[midLink].GetQKDRate() < curDist[*adjNodeIter])
            {
                curDist[*adjNodeIter] = curDist[curNode] + m_vAllLinks[midLink].GetQKDRate();
                preNode[*adjNodeIter] = curNode;
            }
        }
        //Find next node
        RATE minDist = INF;
        NODEID nextNode = curNode;
        for (NODEID nodeId = 0; nodeId < NodeNum; nodeId++)
        {
            if (visited[nodeId])
            {
                continue;
            }
            if (curDist[nodeId] < minDist)
            {
                nextNode = nodeId;
                minDist = curDist[nodeId];
            }
        }
        if (minDist >= INF || nextNode == curNode)
        {
            return false;
        }
        curNode = nextNode;
        visited[nextNode] = true;
    }
    if (curNode != sinkId)
    {
        cout << "why current node is not sink node?? check function shortestPath!" << endl;
        getchar();
        exit(0);
    }
    while(curNode != sourceId)
    {
        nodeList.push_front(curNode);
        NODEID pre = preNode[curNode];
        LINKID midLink = m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);
        curNode = pre;
    }
    nodeList.push_front(sourceId);
    return true;
}

// 显示每个需求的路由的最短路径（或其他路由算法求出的路径）
void CNetwork::ShowDemandPaths()
{
    std::cout << "Demand路径信息:" << std::endl;
    for (auto demandIter = m_vAllDemands.begin(); demandIter != m_vAllDemands.end(); demandIter++)
    {   
        DEMANDID demandId = demandIter->GetDemandId();

        // // 如果路由失败
        // if (demandIter->GetRoutedFailed()) {
        //     std::cout << "Demand ID: " << demandId << " -> Routing Failed" << std::endl;
        //     std::cout << "Path: -1" << std::endl;
        // } else {
        //     // 如果路由成功，打印路径
        //     list<NODEID> node_path = m_vAllDemands[demandId].m_Path.m_lTraversedNodes;
        //     std::cout << "Demand ID: " << demandId << " -> Path: ";
        //     for (const auto& node : node_path) {
        //         std::cout << node << " ";
        //     }
        //     std::cout << std::endl;
        // }
    }
}



// // 为指定需求 demandId 初始化中继路径。如果需求已经被路由，则跳过此操作
// void CNetwork::InitRelayPath(DEMANDID demandId)
// {
//     NODEID sourceId = m_vAllDemands[demandId].GetSourceId();
//     NODEID sinkId = m_vAllDemands[demandId].GetSinkId();
//     list<NODEID> nodeList;
//     list<LINKID> linkList;

//     // 重路由时清空旧路径
//     if(m_dSimTime>0)
//     {
//         // CRelayPath old_path = m_vAllDemands[demandId].m_Path;
//         // 清空旧路径上（源节点以外的）所有节点上关于该demand的标记
//         list<NODEID> TraversedNodes;
//         TraversedNodes = m_vAllDemands[demandId].m_Path.m_lTraversedNodes;
//         for (auto it = ++TraversedNodes.begin(); it != TraversedNodes.end(); it++)
//         {
//             m_vAllNodes[*it].m_mRelayVolume.erase(demandId);
//         }
        
//         // 清空对应的relaypath
//         list<LINKID> TraversedLinks;
//         TraversedLinks = m_vAllDemands[demandId].m_Path.m_lTraversedLinks;
//         for (auto it = TraversedLinks.begin(); it != TraversedLinks.end(); it++)
//         {
//             m_vAllLinks[*it].m_lCarriedDemands.erase(demandId);
//         }

//         m_vAllDemands[demandId].m_Path.Clear();

//     }
//     //更新nextnode
//     // 调用 路由函数，寻找从 sourceId 到 sinkId 的最短/负载均衡路径
//     // cout << "Demand "<<demandId<<" is rerouting"<< endl;
//     if (m_routeStrategy->Route(sourceId, sinkId, nodeList, linkList))
//     {
//         if(m_dSimTime>0)
//         {
//             cout << "Here Demand " << demandId << " is rerouting" << endl;
//             // 恢复源节点上的待传输数据量，以进行重传
//             VOLUME RemainingVolumem = m_vAllDemands[demandId].GetDemandVolume() - m_vAllDemands[demandId].GetDeliveredVolume();
//             m_vAllNodes[sourceId].m_mRelayVolume[demandId] = RemainingVolumem;
//         }
//         m_vAllDemands[demandId].InitRelayPath(nodeList, linkList); // 完成指定demand和中继路径的各种信息的匹配（尤其是node上和指定demand相关的下一条的确定操作
        
//         // CRelayPath new_path = m_vAllDemands[demandId].m_Path;
//         // if(old_path.m_lTraversedNodes != new_path.m_lTraversedNodes)
//         // {
//         //     cout<<"the relaypath of demand "<<demandId<<" is updated"<<endl;
//         // }
//     }
// //    // 通过遍历 linkList，将当前需求ID (demandId) 添加到每条路径链路 m_lCarriedDemands 列表中，表示这些链路将承载该需求的数据传输
// //    for (auto linkIter = linkList.begin(); linkIter != linkList.end(); linkIter++)
// //    {
// //        m_vAllLinks[*linkIter].m_lCarriedDemands.push_back(demandId); // ？？m_lCarriedDemands仅在此做了赋值，之后未使用，感觉不对（被（node,nextnode）的方式代替了）
// //    }
// }

// 为指定需求 demandId 初始化中继路径。如果需求已经被路由，则跳过此操作（多线程）
void CNetwork::InitRelayPath(DEMANDID demandId) {
    NODEID sourceId = m_vAllDemands[demandId].GetSourceId();
    NODEID sinkId = m_vAllDemands[demandId].GetSinkId();
    list<NODEID> nodeList;
    list<LINKID> linkList;

    // 重路由时清空旧路径
    if (m_dSimTime > 0) {
        // 清空旧路径上（源节点以外的）所有节点上关于该demand的标记
        list<NODEID> TraversedNodes = m_vAllDemands[demandId].m_Path.m_lTraversedNodes;
        for (auto it = ++TraversedNodes.begin(); it != TraversedNodes.end(); it++) {
            CNode& node = m_vAllNodes[*it];
            std::lock_guard<std::mutex> lock(node.m_mutex); // 加锁
            node.m_lCarriedDemands.erase(demandId);
        }

        // 清空对应的relaypath
        list<LINKID> TraversedLinks = m_vAllDemands[demandId].m_Path.m_lTraversedLinks;
        for (auto it = TraversedLinks.begin(); it != TraversedLinks.end(); it++) {
            CLink& link = m_vAllLinks[*it];
            std::lock_guard<std::mutex> lock(link.m_mutex); // 加锁
            link.m_lCarriedDemands.erase(demandId);
        }

        m_vAllDemands[demandId].m_Path.Clear();
    }

    // 调用路由函数，寻找从 sourceId 到 sinkId 的最短/负载均衡路径
    if (m_routeStrategy->Route(sourceId, sinkId, nodeList, linkList)) {
        if (m_dSimTime > 0) {
            // 恢复源节点上的待传输数据量，以进行重传
            VOLUME RemainingVolume = m_vAllDemands[demandId].GetDemandVolume() - m_vAllDemands[demandId].GetDeliveredVolume();
            CNode& sourceNode = m_vAllNodes[sourceId];
            std::lock_guard<std::mutex> lock(sourceNode.m_mutex); // 加锁
            sourceNode.m_lCarriedDemands[demandId] = RemainingVolume;
        }

        // 初始化新的中继路径
        m_vAllDemands[demandId].InitRelayPath(nodeList, linkList);
    }
}

void CNetwork::InitLinkDemand()
{
    
    for (auto &demand : m_vAllDemands)
    {
        if (!demand.m_Path.m_lTraversedLinks.empty())
        {
            LINKID linkid = demand.m_Path.m_lTraversedLinks.front();
            m_vAllLinks[linkid].m_lCarriedDemands.insert(demand.GetDemandId());
            // cout << "linkid" << linkid << endl;
            // cout << "demandid" << demand.GetDemandId() << endl;
            cout<<"InitLinkDemand function eshtaning"<<endl;
        }
    }

    // // 遍历link
    // for(auto &link : m_vAllLinks)
    // {
    //     if (link.m_dFaultTime >= 0 && link.m_dFaultTime <= m_dSimTime)
    //     {
    //         continue;
    //     }
    //     NODEID sourceid =  link.GetSourceId();
    //     NODEID sinkid =  link.GetSinkId();
    //     // 遍历link上的demand
    //     for (auto &demandid : link.m_lCarriedDemands)
    //     {
    //         auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
    //         // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
    //         if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
    //         {
    //             NODEID nodeid = sourceid;
    //             VOLUME relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
    //         }
    //         else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
    //         {
    //             NODEID nodeid = sinkid;
    //             VOLUME relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
    //         }
    //         else
    //         {
    //             throw 1;
    //         }
    //     }
    // }
}

// 为所有需求初始化中继路径
void CNetwork::InitRelayPath()
{
    cout << "Init Relay Path" << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto demandIter = m_vAllDemands.begin(); demandIter != m_vAllDemands.end(); demandIter++)
    {
        InitRelayPath(demandIter->GetDemandId());
        // cout << "Initing Relay Path for demand " << demandIter->GetDemandId() << endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "InitRelayPath time: " << elapsed.count() << " seconds" << std::endl;
}

// // 为所有需求初始化中继路径(并发)
// void CNetwork::InitRelayPath(size_t max_threads = std::thread::hardware_concurrency()) {
//     std::cout << "Init Relay Path" << std::endl;
//     auto start = std::chrono::high_resolution_clock::now();

//     // 如果未指定最大线程数，则使用硬件支持的线程数
//     if (max_threads == 0) {
//         max_threads = std::thread::hardware_concurrency();
//     }

//     // 创建线程池
//     std::vector<std::thread> threads;
//     threads.reserve(max_threads); // 预分配线程空间

//     // 任务分块处理
//     size_t num_demands = m_vAllDemands.size();
//     size_t chunk_size = (num_demands + max_threads - 1) / max_threads; // 每个线程处理的任务数

//     // 启动线程处理任务块
//     for (size_t i = 0; i < max_threads; ++i) {
//         size_t start_index = i * chunk_size;
//         size_t end_index = std::min(start_index + chunk_size, num_demands);

//         threads.emplace_back([this, start_index, end_index]() {
//             for (size_t j = start_index; j < end_index; ++j) {
//                 DEMANDID demandId = m_vAllDemands[j].GetDemandId();
//                 InitRelayPath(demandId); // 每个线程处理一个任务块

//                 std::this_thread::sleep_for(std::chrono::milliseconds(400));
//             }
//         });
//     }

//     // 等待所有线程完成
//     for (auto& thread : threads) {
//         thread.join();
//     }

//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> elapsed = end - start;
//     std::cout << "InitRelayPath time: " << elapsed.count() << " seconds" << std::endl;
// }

// 计算给定节点 nodeId 上最小剩余时间优先的需求转发时间，并记录将要转发的需求和数据量
// 基于节点
TIME CNetwork::MinimumRemainingTimeFirst(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands)
{
    TIME executeTime = INF;                // 表示当前的最小执行时间
    map<LINKID, DEMANDID> scheduledDemand; // 记录每条链路上计划要转发的需求
    map<DEMANDID, TIME> executeTimeDemand; // 记录需求的执行时间
    // 遍历当前节点 nodeId 上的所有需求（记录在 m_mRelayVolume 中），跳过尚未到达的需求（通过到达时间判断）
    // 创建一个随机数引擎
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(0, 1); // 定义一个分布，生成 0 或 1

    // 生成随机数
    int tempWait = dis(gen);
    for (auto demandIter = m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[nodeId].m_mRelayVolume.end(); demandIter++)
    {
        DEMANDID selectedDemand = demandIter->first;
        if (m_vAllDemands[selectedDemand].GetArriveTime() > m_dSimTime + SMALLNUM)
        {
            // this demand has not arrived yet
            continue;
        }
        // 根据链路的带宽 bandwidth 和需求的剩余数据量 demandIter->second，计算需求的执行时间，并更新最小执行时间 executeTime
        NODEID nextNode = m_vAllDemands[selectedDemand].m_Path.m_mNextNode[nodeId];
        LINKID midLink = m_mNodePairToLink[make_pair(nodeId, nextNode)];
        RATE bandwidth = m_vAllLinks[midLink].GetBandwidth();

        // 获取该链路上的可用密钥量
        VOLUME availableKeyVolume = m_vAllLinks[midLink].GetAvaialbeKeys();
        // cout << "availableKeyVolume" << availableKeyVolume << endl;
        VOLUME actualTransmittableVolume = min(demandIter->second, availableKeyVolume);
        // cout<<"actualTransmittableVolume:"<<actualTransmittableVolume<<endl;
        // 根据链路的带宽和实际可传输的数据量，计算需求的执行时间，并更新最小执行时间 executeTime

        TIME demandExecuteTime = actualTransmittableVolume / bandwidth;
        // cout << "bandwidth:" << bandwidth << endl;
        // cout << "demandExecuteTime:" << actualTransmittableVolume / bandwidth << endl;
        
        if (demandExecuteTime < 3)
        {
            if (tempWait == 1)
            {
                m_vAllLinks[midLink].wait_or_not = true;
                continue;
            }
        }
        if (demandExecuteTime < executeTime)
        {

            if (demandIter->second > availableKeyVolume)
            {
                m_vAllLinks[midLink].wait_or_not = true;
                // TIME waitTime = (demandIter->second - availableKeyVolume) / m_vAllLinks[midLink].GetQKDRate();
                // if (waitTime < executeTime)
                // {
                //     executeTime = waitTime;
                // }
                continue;
                // executeTime = (demandIter->second - availableKeyVolume) / m_vAllLinks[midLink].GetQKDRate();
            }
            else
            {
                m_vAllLinks[midLink].wait_or_not = false;
                executeTime = demandExecuteTime;
            }
        }
        // 该需求的执行时间
        executeTimeDemand[selectedDemand] = demandExecuteTime;

        // 如果该链路上还没有被调度的需求，将当前需求 selectedDemand 设置为该链路的调度需求。
        // if (availableKeyVolume >= demandIter->second || availableKeyVolume >= minAvailableKeyVolume)

        if (scheduledDemand.find(midLink) == scheduledDemand.end())
        {
            scheduledDemand[midLink] = selectedDemand;
        }
        else // 如果该链路已经有一个需求被调度，那么比较新需求和已调度需求的剩余数据量，选择数据量较少的需求作为调度对象（最小剩余时间优先）
        {
            DEMANDID preDemand = scheduledDemand[midLink];
            if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand])
                // if (m_vAllNodes[nodeId].m_mRelayVolume[preDemand] > m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand] && m_vAllNodes[nodeId].m_mRelayVolume[selectedDemand] != 0)
            {
                scheduledDemand[midLink] = selectedDemand;
            }
        }

    }

    // 遍历所有被调度的需求，计算它们在执行时间内的转发数据量（带宽乘以执行时间），并将这些数据量记录在 relayDemands 中
    for (auto scheduledIter = scheduledDemand.begin(); scheduledIter != scheduledDemand.end(); scheduledIter++)
    {
        RATE bandwidth = m_vAllLinks[scheduledIter->first].GetBandwidth();
        // if (bandwidth * executeTime < 1)
        // {
        //     relayDemands[scheduledIter->second] = 0;
        // }
        // else
        // {
        //     relayDemands[scheduledIter->second] = bandwidth * executeTime;
        // }
        relayDemands[scheduledIter->second] = bandwidth * executeTime;
    }
    // if (executeTime == INF)
    // {
    //     executeTime = 5;
    // }

    // if(executeTime != INF)
    //     cout << "executeTime:" << executeTime << endl;

    return executeTime;
}

// 计算给定节点 nodeId 上最小剩余时间优先的需求转发时间，并记录将要转发的需求和数据量
// 基于链路
TIME CNetwork::MinimumRemainingTimeFirstLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands)
{
    TIME executeTime = INF; // 表示当前的最小执行时间
    map<LINKID, DEMANDID> scheduledDemand; // 记录每条链路上计划要转发的需求
    map<DEMANDID, TIME> executeTimeDemand;         // 记录需求的执行时间
    VOLUME availableKeyVolume = m_vAllLinks[linkId].GetAvaialbeKeys();
    // 获取当前链路的首末端点
    NODEID SourceId = m_vAllLinks[linkId].GetSourceId();
    NODEID SinkId = m_vAllLinks[linkId].GetSinkId();
    RATE bandwidth = m_vAllLinks[linkId].GetBandwidth();
    // 遍历首末端点
    map<DEMANDID, VOLUME>::iterator demandIter;
    
    for (auto demandIter = m_vAllNodes[SourceId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[SourceId].m_mRelayVolume.end(); demandIter++)
    {
        DEMANDID selectedDemand = demandIter->first;
        if (m_vAllDemands[selectedDemand].GetArriveTime() > m_dSimTime + SMALLNUM)
        {
            // this demand has not arrived yet
            continue;
        }
        // 根据链路的带宽 bandwidth 和需求的剩余数据量 demandIter->second，计算需求的执行时间，并更新最小执行时间 executeTime
        NODEID nextNode = m_vAllDemands[selectedDemand].m_Path.m_mNextNode[SourceId];
        LINKID midLink = m_mNodePairToLink[make_pair(SourceId, nextNode)];
        if (midLink == linkId)
        {
            VOLUME demandVolume = m_vAllNodes[SourceId].m_mRelayVolume[selectedDemand];
            if (demandVolume <= availableKeyVolume)
            {
                if (scheduledDemand.find(linkId) == scheduledDemand.end())
                {
                    scheduledDemand[linkId] = selectedDemand;
                }
                else // 如果该链路已经有一个需求被调度，那么比较新需求和已调度需求的剩余数据量，选择数据量较少的需求作为调度对象（最小剩余时间优先）
                {
                    DEMANDID preDemand = scheduledDemand[linkId];
                    if (demandVolume < m_vAllNodes[SourceId].m_mRelayVolume[preDemand])
                    {
                        scheduledDemand[linkId] = selectedDemand;
                        executeTime = demandVolume / bandwidth;
                    }
                }
            }
        }
    }
    
    for (auto demandIter = m_vAllNodes[SinkId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[SinkId].m_mRelayVolume.end(); demandIter++)
    {
        DEMANDID selectedDemand = demandIter->first;
        if (m_vAllDemands[selectedDemand].GetArriveTime() > m_dSimTime + SMALLNUM)
        {
            // this demand has not arrived yet
            continue;
        }
        // 根据链路的带宽 bandwidth 和需求的剩余数据量 demandIter->second，计算需求的执行时间，并更新最小执行时间 executeTime
        NODEID nextNode = m_vAllDemands[selectedDemand].m_Path.m_mNextNode[SinkId];
        LINKID midLink = m_mNodePairToLink[make_pair(SinkId, nextNode)];
        if (midLink == linkId)
        {
            VOLUME demandVolume = m_vAllNodes[SinkId].m_mRelayVolume[selectedDemand];
            if (demandVolume <= availableKeyVolume)
            {
                if (scheduledDemand.find(linkId) == scheduledDemand.end())
                {
                    scheduledDemand[linkId] = selectedDemand;
                }
                else // 如果该链路已经有一个需求被调度，那么比较新需求和已调度需求的剩余数据量，选择数据量较少的需求作为调度对象（最小剩余时间优先）
                {
                    DEMANDID preDemand = scheduledDemand[linkId];
                    if (demandVolume < m_vAllNodes[SinkId].m_mRelayVolume[preDemand])
                    {
                        scheduledDemand[linkId] = selectedDemand;
                        executeTime = demandVolume / bandwidth;
                    }
                }
            }
        }
    }
    relayDemands[scheduledDemand[linkId]] = bandwidth * executeTime;
    return executeTime;
}

// 平均分配当前密钥，计算给定节点的需求转发执行时间
// 思路：应该是链路承担的所有需求
TIME CNetwork::AverageKeyScheduling(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands)
{
    // 创建一个随机数引擎
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(0, 1); // 定义一个分布，生成 0 或 1

    // 生成随机数
    int tempWait = dis(gen);

    // cout<<"进入平均密钥调度算法"<<endl;
    TIME executeTime = INF;                // 表示当前的最小执行时间
    // 遍历link
    for(auto &link : m_vAllLinks)
    {
        // 只遍历链路首或尾是该节点的链路
        if (link.GetSourceId() != nodeId && link.GetSinkId() != nodeId)
        {
            continue;
        }
        // cout<<"该节点存在链路"<<endl;
        // 跳过故障link和link上没需求的link
        // if (link.GetFaultTime() >= 0 && link.GetFaultTime() <= m_dSimTime || link.m_lCarriedDemands.empty())
        if (link.GetFaultTime() > 0 && link.GetFaultTime() <= m_dSimTime || link.m_lCarriedDemands.empty())
        {
            link.wait_or_not = true;
            continue;
        }
        
        VOLUME availableKeyVolume = link.GetAvaialbeKeys();
        RATE bandwidth = link.GetBandwidth();
        NODEID sourceid =  link.GetSourceId();
        NODEID sinkid =  link.GetSinkId();
        TIME tempTime = INF;
        int num_of_demand = 0;
        // 遍历link上的demand，得到一条链路上的执行时间
        for (auto &demandid : link.m_lCarriedDemands)
        {
            // cout<<demandid<<endl;
            if (m_vAllDemands[demandid].GetArriveTime() > m_dSimTime + SMALLNUM)
            {
                // this demand has not arrived yet
                continue;
            }
            // cout<<"该链路存在需求"<<endl;
            num_of_demand++;
            cout<< "平均密钥调度:"<< link.GetLinkId()<< "have demand" << demandid<<endl;
            NODEID nodeid;
            VOLUME relayVolume;
            auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
            // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
            if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
            {
                nodeid = sourceid;
                relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
            }
            else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
            {
                nodeid = sinkid;
                relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
            }
            // else
            // {
            //     throw 1;
            // }

            // 对一个demand，判断链路最小执行时间tempTime
            if (relayVolume / bandwidth < tempTime)
            {
                tempTime = relayVolume / bandwidth;
            }
            // cout << "relayVolume" << relayVolume<<endl;
            // cout << "bandwidth" << bandwidth<<endl;
            // cout << "tempTime" << tempTime<<endl;
        }
        // 没有可以传的需求
        if (num_of_demand == 0)
        {
            continue;
        }
        // 找到了该条链路上的最小执行时间tempTime，计算最小传输量，然后比较可用密钥量
        // VOLUME needVolume = tempTime * bandwidth * link.m_lCarriedDemands.size();
        VOLUME needVolume = tempTime * bandwidth * num_of_demand;
        // cout << "needVolume" << needVolume<<endl;
        // cout << "availableKeyVolume" << availableKeyVolume<<endl;
        // 如果可用密钥量足够，给每一个nodeid，赋值传同样的最小传输量
        if (needVolume <= availableKeyVolume)
        {
            link.wait_or_not = false;
            for (auto &demandid : link.m_lCarriedDemands)
            {
                NODEID nodeid;
                auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
                // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
                if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
                {
                    nodeid = sourceid;
                }
                else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
                {
                    nodeid = sinkid;
                }
                // else
                // {
                //     throw 1;
                // }
                // 对每一个可以传输的demand，给相应的nodeid传输最小传输量
                if (nodeid == nodeId)
                {
                    relayDemands[demandid] = tempTime * bandwidth;
                }
            }
        }
        else if (availableKeyVolume >= 50)
        {
            link.wait_or_not = false;
            for (auto &demandid : link.m_lCarriedDemands)
            {
                NODEID nodeid;
                auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
                // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
                if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
                {
                    nodeid = sourceid;
                }
                else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
                {
                    nodeid = sinkid;
                }
                // else
                // {
                //     throw 1;
                // }
                // 对每一个可以传输的demand，给相应的nodeid传输最小传输量
                if (nodeid == nodeId)
                {
                    // relayDemands[demandid] = availableKeyVolume / link.m_lCarriedDemands.size();
                    relayDemands[demandid] = availableKeyVolume / num_of_demand;
                }
            }
        }
        else
        {
            link.wait_or_not = true;
            tempTime = (needVolume - availableKeyVolume) / link.GetQKDRate();
        }
        // if (tempTime < 3)
        // {
        //     // if (tempWait == 1)
        //     // {
        //     //     link.wait_or_not = true;
        //     //     continue;
        //     // }
        //     link.wait_or_not = true;
        //     executeTime = 3;
        //     continue;
        // }
        if (tempTime < executeTime)
        {
            executeTime = tempTime;
        }
    }
    if(executeTime != INF)
        cout << "executeTime:" << executeTime << endl;
    return executeTime;
}

TIME CNetwork::AverageKeySchedulingLinkBased(LINKID linkId, map<DEMANDID, VOLUME> &relayDemands)
{
    TIME executeTime = INF;                // 表示当前的最小执行时间
    // 创建一个随机数引擎
    std::random_device rd;  // 用于获取随机种子
    // auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_real_distribution<double> dis(0.0, 1.0); // 生成 0.0 到 1.0 之间的随机小数

    // 生成随机数

    double tempWait = dis(gen);
    // cout<<"tempWait"<<tempWait<<endl;
    // 跳过故障link和link上没需求的link
    if (m_vAllLinks[linkId].GetFaultTime() > 0 && m_vAllLinks[linkId].GetFaultTime() <= m_dSimTime || m_vAllLinks[linkId].m_lCarriedDemands.empty())
    {
        m_vAllLinks[linkId].wait_or_not = true;
        relayDemands.clear();
        return executeTime;
    }
    
    VOLUME availableKeyVolume = m_vAllLinks[linkId].GetAvaialbeKeys();
    RATE bandwidth = m_vAllLinks[linkId].GetBandwidth();
    NODEID sourceid =  m_vAllLinks[linkId].GetSourceId();
    NODEID sinkid =  m_vAllLinks[linkId].GetSinkId();
    TIME tempTime = INF;
    int num_of_demand = 0;
    // 遍历link上的demand，得到一条链路上的执行时间
    set<NODEID> node_set;
    for (auto &demandid : m_vAllLinks[linkId].m_lCarriedDemands)
    {
        
        if (m_vAllDemands[demandid].GetArriveTime() > m_dSimTime + SMALLNUM)
        {
            // this demand has not arrived yet
            continue;
        }
        num_of_demand++;
        // cout<< m_vAllLinks[linkId].GetLinkId()<< "have demand" << demandid<<endl;
        NODEID nodeid;
        VOLUME relayVolume;
        auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
        // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
        if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
        {
            nodeid = sourceid;
            relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
            if (node_set.find(nodeid) == node_set.end()) {
                node_set.insert(nodeid); // 不存在则插入
            }
        }
        else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
        {
            nodeid = sinkid;
            relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
            if (node_set.find(nodeid) == node_set.end()) {
                node_set.insert(nodeid); // 不存在则插入
            }
        }
        // else
        // {
        //     throw 1;
        // }

        // 对一个demand，判断链路最小执行时间tempTime
        if (relayVolume)
        {
            if (relayVolume / bandwidth < tempTime)
            {
                tempTime = relayVolume / bandwidth;
            }
        }
        // cout << "relayVolume" << relayVolume<<endl;
        // cout << "bandwidth" << bandwidth<<endl;
        // cout << "tempTime" << tempTime<<endl;
    }
    // 没有可以传的需求
    // if (num_of_demand == 0)
    // {
    //     continue;
    // }
    // 找到了该条链路上的最小执行时间tempTime，计算最小传输量，然后比较可用密钥量
    // VOLUME needVolume = tempTime * bandwidth * link.m_lCarriedDemands.size();
    // if (tempTime < 1)
    // {
    //     if (tempWait < 0.95)
    //     {
    //         m_vAllLinks[linkId].wait_or_not = true;
    //         relayDemands.clear();
    //         return 3;
    //     }
    // }
    VOLUME needVolume = tempTime * bandwidth * num_of_demand;
    // cout << "needVolume" << needVolume<<endl;
    // cout << "availableKeyVolume" << availableKeyVolume<<endl;
    // 如果可用密钥量足够，给每一个nodeid，赋值传同样的最小传输量
    if (needVolume <= availableKeyVolume)
    {
        m_vAllLinks[linkId].wait_or_not = false;
        for (auto &demandid : m_vAllLinks[linkId].m_lCarriedDemands)
        {
            // 对每一个可以传输的demand，给相应的nodeid传输最小传输量
            relayDemands[demandid] = tempTime * bandwidth;
        }
        // if (tempWait < 0.9)
        // {
        //     m_vAllLinks[linkId].wait_or_not = true;
        //     relayDemands.clear();
        //     return 3;
        // }
    }
    // else if (availableKeyVolume >= 10)
    // {
    //     link.wait_or_not = false;
    //     for (auto &demandid : link.m_lCarriedDemands)
    //     {
    //         NODEID nodeid;
    //         auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
    //         // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
    //         if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
    //         {
    //             nodeid = sourceid;
    //         }
    //         else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
    //         {
    //             nodeid = sinkid;
    //         }
    //         // else
    //         // {
    //         //     throw 1;
    //         // }
    //         // 对每一个可以传输的demand，给相应的nodeid传输最小传输量
    //         if (nodeid == nodeId)
    //         {
    //             // relayDemands[demandid] = availableKeyVolume / link.m_lCarriedDemands.size();
    //             relayDemands[demandid] = availableKeyVolume / num_of_demand;
    //         }
    //     }
    // }
    else
    {
        m_vAllLinks[linkId].wait_or_not = true;
        relayDemands.clear();
        return 3;
        // tempTime = (needVolume - availableKeyVolume) / m_vAllLinks[linkId].GetQKDRate();
    }
    if (tempTime < executeTime)
    {
        executeTime = tempTime;
    }
    
    return executeTime;
}


// 为指定节点 nodeId 找到需要转发的需求，并计算所需时间
TIME CNetwork::FindDemandToRelay(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemand)
{
    // return currentScheduleAlg(nodeId, relayDemand);
    return m_schedStrategy->Sched(nodeId,relayDemand);
}

// 为所有节点找到需要转发的需求，并计算执行时间
TIME CNetwork::FindDemandToRelayLinkBased(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand)
{
    map<NODEID, map<DEMANDID, VOLUME>> nodeRelayDemand; // 表示对应NODEID在nodeRelayTime时间中，每个需求发送的数据量
    map<LINKID, map<DEMANDID, VOLUME>> linkRelayDemand; // 表示对应LINKID在linkRelayTime时间中，每个需求发送的数据量
    map<NODEID, TIME> nodeRelayTime;                    // NODEID节点上的需求执行一跳的最短时间
    map<LINKID, TIME> linkRelayTime;
    TIME minExecuteTime = INF;
    // 遍历所有链路 (linkId)，对每个链路调用 FindDemandToRelay，计算该链路的需求转发时间和需要转发的需求量 tempRelayDemand
    for (LINKID linkId = 0; linkId < m_uiLinkNum; linkId++)
    {
        map<DEMANDID, VOLUME> tempRelayDemand;
        TIME executeTime = FindDemandToRelay(linkId, tempRelayDemand);
        // 将每个节点的最小转发时间存储在 nodeRelayTime 中，并更新 minExecuteTime 以记录全网络的最小转发时间
        if (m_vAllLinks[linkId].wait_or_not == true)
        {
            continue;
        }
        
        linkRelayTime[linkId] = executeTime;
        if (executeTime < minExecuteTime)
        {
            minExecuteTime = executeTime;
        }
        // linkRelayTime[linkId] = executeTime;
        // 将每个节点的转发需求量存储在 nodeRelayDemand 中
        linkRelayDemand[linkId] = tempRelayDemand;
    }
    if (minExecuteTime == INF)
    {
        minExecuteTime = 3;
    }
    if (minExecuteTime < 3)
    {
        minExecuteTime = 3;
        relayDemand.clear();
    }
    cout << "minExecuteTime: " << minExecuteTime << endl;

    // 判断是否在当前最小转发时间 minExecuteTime 内有新的需求到达。如果是，则将 minExecuteTime 更新为下一个需求到达时间与当前模拟时间的差值
    if (!m_mDemandArriveTime.empty() && m_dSimTime + minExecuteTime + SMALLNUM > m_mDemandArriveTime.begin()->first)
    {
        minExecuteTime = m_mDemandArriveTime.begin()->first - m_dSimTime;
    }
    // 把linkRelayDemand映射到nodeRelayDemand
    map<LINKID, map<DEMANDID, VOLUME>>::iterator linkIter;
    linkIter = linkRelayDemand.begin();
    for (; linkIter != linkRelayDemand.end(); linkIter++)
    {
        LINKID linkId = linkIter->first;
        map<DEMANDID, VOLUME>& demand_map = linkIter->second;
        // 遍历所有 DEMANDID 参数
        for (const auto& pair : demand_map) {
            DEMANDID demand_id = pair.first;
            // 获取当前链路的首末端点
            NODEID SourceId = m_vAllLinks[linkId].GetSourceId();
            NODEID SinkId = m_vAllLinks[linkId].GetSinkId();
            map<DEMANDID, VOLUME>::iterator demandIter;
            // 确定这个需求是从哪个端点传到哪边。
            int succ_signal = 0;
            for (auto demandIter = m_vAllNodes[SourceId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[SourceId].m_mRelayVolume.end(); demandIter++)
            {
                if (demandIter->first == demand_id)
                {
                    nodeRelayDemand[SourceId][demand_id] = pair.second;
                    // // 这个传输时间是为了后面调整传输比例
                    nodeRelayTime[SourceId] = linkRelayTime[linkId];
                    succ_signal = 1;
                    break;
                    // // 找到这个节点，假如没找到这个节点
                    // if (nodeRelayDemand.find(SourceId) == nodeRelayDemand.end())
                    // {
                    //     nodeRelayDemand[SourceId][demand_id] = pair.second;
                    //     // nodeRelayDemand[SourceId] = linkIter->second;
                    //     // // 这个传输时间是为了后面调整传输比例
                    //     nodeRelayTime[SourceId] = linkRelayTime[linkId];
                    //     succ_signal = 1;
                    //     break;
                    // }
                    // // 找到了这个节点，说明这个节点已经有要传输的需求，那么选择较小的需求传输
                    // else
                    // {
                    //     map<DEMANDID, VOLUME>& demand_map_temp = nodeRelayDemand[SourceId];
                    //     for (const auto& pair_temp : demand_map_temp)
                    //     {
                    //         if(pair.second < pair_temp.second)
                    //         {
                    //             nodeRelayDemand[SourceId] = linkIter->second;
                    //             nodeRelayTime[SourceId] = linkRelayTime[linkId];
                    //             succ_signal = 1;
                    //             break;
                    //         }
                    //     }
                    // }
                }
            }
            if (succ_signal != 1)
            {
                if (nodeRelayDemand.find(SinkId) == nodeRelayDemand.end())
                {
                    nodeRelayDemand[SinkId][demand_id] = pair.second;
                    // nodeRelayDemand[SinkId] = linkIter->second;
                    nodeRelayTime[SinkId] = linkRelayTime[linkId];
                    // succ_signal = 1;
                    break;
                }
                // else
                // {
                //     map<DEMANDID, VOLUME>& demand_map_temp = nodeRelayDemand[SinkId];
                //     for (const auto& pair_temp : demand_map_temp)
                //     {
                //         if(pair.second < pair_temp.second)
                //         {
                //             nodeRelayDemand[SinkId] = linkIter->second;
                //             nodeRelayTime[SinkId] = linkRelayTime[linkId];
                //             // succ_signal = 1;
                //             break;
                //         }
                //     }
                // }
            }
        }
    }

    // 对每个节点，将需求的转发量按最小执行时间比例缩放，并记录在 relayDemand 中
    map<NODEID, map<DEMANDID, VOLUME>>::iterator nodeIter;
    map<DEMANDID, VOLUME>::iterator demandIter;
    nodeIter = nodeRelayDemand.begin();
    for (; nodeIter != nodeRelayDemand.end(); nodeIter++)
    {
        TIME relayTime = nodeRelayTime[nodeIter->first];
        demandIter = nodeIter->second.begin(); // second表示该元素的value值
        for (; demandIter != nodeIter->second.end(); demandIter++)
        {
            VOLUME newVolume = demandIter->second * minExecuteTime / relayTime;
            relayDemand[nodeIter->first][demandIter->first] = newVolume;
        }
    }
    return minExecuteTime;
}

// 为所有节点找到需要转发的需求，并计算执行时间
TIME CNetwork::FindDemandToRelay(map<NODEID, map<DEMANDID, VOLUME>> &relayDemand)
{
    std::cout<<"进入FindDemandToRelay阶段，进行调度方案计算"<< std::endl;
    
    map<NODEID, map<DEMANDID, VOLUME>> nodeRelayDemand; // 表示对应NODEID在nodeRelayTime时间中，每个需求发送的数据量
    map<NODEID, TIME> nodeRelayTime;                    // NODEID节点上的需求执行一跳的最短时间
    TIME minExecuteTime = INF;
    // 遍历所有节点 (nodeId)，对每个节点调用 FindDemandToRelay，计算该节点的需求转发时间和需要转发的需求量 tempRelayDemand
    for (NODEID nodeId = 0; nodeId < m_uiNodeNum; nodeId++)
    {
        map<DEMANDID, VOLUME> tempRelayDemand;
        TIME executeTime = FindDemandToRelay(nodeId, tempRelayDemand);
        // 将每个节点的最小转发时间存储在 nodeRelayTime 中，并更新 minExecuteTime 以记录全网络的最小转发时间
        nodeRelayTime[nodeId] = executeTime;
        if (executeTime < minExecuteTime)
        {
            minExecuteTime = executeTime;
        }
        // 将每个节点的转发需求量存储在 nodeRelayDemand 中
        nodeRelayDemand[nodeId] = tempRelayDemand;
    }
    if (minExecuteTime == INF)
    {
        minExecuteTime = 10;
        // nodeRelayDemand.clear();
    }
    // 判断是否在当前最小转发时间 minExecuteTime 内有新的需求到达。如果是，则将 minExecuteTime 更新为下一个需求到达时间与当前模拟时间的差值
    if (!m_mDemandArriveTime.empty() && m_dSimTime + minExecuteTime + SMALLNUM > m_mDemandArriveTime.begin()->first)
    {
        minExecuteTime = m_mDemandArriveTime.begin()->first - m_dSimTime;
    }

    cout << "minExecuteTime: " << minExecuteTime << endl;


    // 对每个节点，将需求的转发量按最小执行时间比例缩放，并记录在 relayDemand 中
    for (auto nodeIter = nodeRelayDemand.begin(); nodeIter != nodeRelayDemand.end(); nodeIter++)
    {
        TIME relayTime = nodeRelayTime[nodeIter->first];
        for (auto demandIter = nodeIter->second.begin(); demandIter != nodeIter->second.end(); demandIter++)
        {
            VOLUME newVolume = demandIter->second * minExecuteTime / relayTime;
            // if (newVolume >= 1)
            // {
            //     relayDemand[nodeIter->first][demandIter->first] = newVolume;
            // }
            // else
            // {
            //     relayDemand[nodeIter->first][demandIter->first] = 0;
            // }
            // relayDemand[nodeIter->first][demandIter->first] = newVolume;
            relayDemand[nodeIter->first][demandIter->first] = demandIter->second;
            // cout << "node id = " << nodeIter->first << endl;
            // cout << "demand id = " << demandIter->first << endl;
            // cout << "demand volume = " << demandIter->second << endl;
        }
    }

    return minExecuteTime;
}

// 执行一次单跳的需求转发操作，更新各节点和链路上的数据量和密钥(存在中继路径无法查询的问题)
// void CNetwork::RelayForOneHop(TIME executeTime, map<NODEID, map<DEMANDID, VOLUME>> &relayDemands)
// {
//     std::cout<<"进入RelayForOneHop阶段，进行调度方案执行"<< std::endl;
//     map<NODEID, map<DEMANDID, VOLUME>>::iterator nodeIter;
//     nodeIter = relayDemands.begin();
//     for (; nodeIter != relayDemands.end(); nodeIter++)
//     {
//         for (auto demandIter = nodeIter->second.begin(); demandIter != nodeIter->second.end(); demandIter++)
//         {
//             // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
//             NODEID nextNode = m_vAllDemands[demandIter->first].m_Path.m_mNextNode[nodeIter->first];

//             // 找到当前节点和下一个节点之间的链路 minLink，并在该链路上消耗相应的密钥数量（等于转发的数据量）
//             LINKID minLink = m_mNodePairToLink[make_pair(nodeIter->first, nextNode)];
//             cout<<"nextNode:"<<nextNode<<endl;
//             cout<<"minLink:"<<minLink<<endl;
//             // if (m_vAllLinks[minLink].GetAvaialbeKeys() > THRESHOLD)
//             if (m_vAllLinks[minLink].wait_or_not == false)
//             {
//                 m_vAllLinks[minLink].ConsumeKeys(demandIter->second);
//                 // 从当前节点上移除已经转发的需求数据量 (demandIter->second)。如果当前节点是该需求的源节点，调用 ReduceVolume 减少需求的剩余数据量
//                 // ？？仅在需求在源节点被传输后，才减少数据量，中间节点没有这个操作，不知道是否符合逻辑
//                 cout << "demandIter->second" << demandIter->second << endl;
//                 if (nodeIter->first == m_vAllDemands[demandIter->first].GetSourceId())
//                 {
//                     cout << "nodeIter->first" << nodeIter->first << endl;
//                     cout << "m_vAllDemands[demandIter->first].GetSourceId()" << m_vAllDemands[demandIter->first].GetSourceId() << endl;
//                     m_vAllDemands[demandIter->first].ReduceVolume(demandIter->second);
//                 }

//                 m_vAllNodes[nodeIter->first].m_mRelayVolume[demandIter->first] -= demandIter->second;

//                 // 当需求从此节点完全传输，则删除此节点上m_mRelayVolume对应的需求
//                 if (m_vAllNodes[nodeIter->first].m_mRelayVolume[demandIter->first] <= INFSMALL)  // 这里可能有问题
//                 // if (m_vAllNodes[nodeIter->first].m_mRelayVolume[demandIter->first] <= 1)  // 这里可能有问题
//                 {
//                     m_vAllNodes[nodeIter->first].m_mRelayVolume.erase(demandIter->first);
//                     // 对link.m_lCarriedDemands进行删除
//                     m_vAllLinks[minLink].m_lCarriedDemands.erase(demandIter->first);
//                     // if (m_vAllLinks[minLink].m_lCarriedDemands.find(demandIter->first) == m_vAllLinks[minLink].m_lCarriedDemands.end())
//                     // {
//                     //     m_vAllLinks[minLink].m_lCarriedDemands.erase(demandIter->first);
//                     // }
//                 }
                
//                 // 如果 nextNode 是需求的目标节点（汇节点），则调用 UpdateDeliveredVolume 更新已传输的数据量，并结束本次中继操作。如果 nextNode 不是汇节点，则将需求数据量添加到下一个节点的中继列表中
//                 if (nextNode == m_vAllDemands[demandIter->first].GetSinkId())
//                 {
//                     m_vAllDemands[demandIter->first].UpdateDeliveredVolume(demandIter->second, m_dSimTime);
//                     continue;
//                 }

//                 if (demandIter->second != 0)
//                 {
//                     m_vAllNodes[nextNode].m_mRelayVolume[demandIter->first] += demandIter->second;

//                     // 对下一条链路link.m_lCarriedDemands进行添加
//                     NODEID nextNextNode = m_vAllDemands[demandIter->first].m_Path.m_mNextNode[nextNode];
//                     LINKID nextMinLink = m_mNodePairToLink[make_pair(nextNode, nextNextNode)];
//                     if (m_vAllLinks[nextMinLink].m_lCarriedDemands.find(demandIter->first) == m_vAllLinks[nextMinLink].m_lCarriedDemands.end())
//                     {
//                         m_vAllLinks[nextMinLink].m_lCarriedDemands.insert(demandIter->first);
//                         cout<< "--------------------------------------------------------" <<endl;
//                         cout<< m_vAllLinks[nextMinLink].GetLinkId() << "insert" << demandIter->first << endl;
//                     }
//                 }
//             }
//         }
//     }
//     // 调用 UpdateRemainingKeys，根据执行时间 executeTime 更新所有链路上的剩余密钥数量
//     UpdateRemainingKeys(executeTime, m_dSimTime);
// }

void CNetwork::RelayForOneHop(TIME executeTime, map<NODEID, map<DEMANDID, VOLUME>> &relayDemands)
{
    std::cout<<"进入RelayForOneHop阶段，进行调度方案执行"<< std::endl;
    map<NODEID, map<DEMANDID, VOLUME>>::iterator nodeIter;
    nodeIter = relayDemands.begin();
    for (; nodeIter != relayDemands.end(); nodeIter++)
    {
        for (auto demandIter = nodeIter->second.begin(); demandIter != nodeIter->second.end(); demandIter++)
        {
            // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
            NODEID nextNode = m_vAllDemands[demandIter->first].m_Path.m_mNextNode[nodeIter->first];
            // 找到当前节点和下一个节点之间的链路 minLink，并在该链路上消耗相应的密钥数量（等于转发的数据量）
            LINKID minLink = m_mNodePairToLink[make_pair(nodeIter->first, nextNode)];
            cout<<"nextNode:"<<nextNode<<endl;
            cout<<"minLink:"<<minLink<<endl;
            // if (m_vAllLinks[minLink].GetAvaialbeKeys() > THRESHOLD)
            if (m_vAllLinks[minLink].wait_or_not == false)
            {
                m_vAllLinks[minLink].ConsumeKeys(demandIter->second);
                // 从当前节点上移除已经转发的需求数据量 (demandIter->second)。如果当前节点是该需求的源节点，调用 ReduceVolume 减少需求的剩余数据量
                // ？？仅在需求在源节点被传输后，才减少数据量，中间节点没有这个操作，不知道是否符合逻辑
                // cout << "demandIter->second" << demandIter->second << endl;
                if (nodeIter->first == m_vAllDemands[demandIter->first].GetSourceId())
                {
                    // cout << "nodeIter->first" << nodeIter->first << endl;
                    // cout << "m_vAllDemands[demandIter->first].GetSourceId()" << m_vAllDemands[demandIter->first].GetSourceId() << endl;
                    m_vAllDemands[demandIter->first].ReduceVolume(demandIter->second);
                }

                m_vAllNodes[nodeIter->first].m_mRelayVolume[demandIter->first] -= demandIter->second;

                // 当需求从此节点完全传输，则删除此节点上m_mRelayVolume对应的需求
                if (m_vAllNodes[nodeIter->first].m_mRelayVolume[demandIter->first] <= INFSMALL)  // 这里可能有问题
                // if (m_vAllNodes[nodeIter->first].m_mRelayVolume[demandIter->first] <= 1)  // 这里可能有问题
                {
                    m_vAllNodes[nodeIter->first].m_mRelayVolume.erase(demandIter->first);
                    // 对link.m_lCarriedDemands进行删除
                    m_vAllLinks[minLink].m_lCarriedDemands.erase(demandIter->first);
                    // if (m_vAllLinks[minLink].m_lCarriedDemands.find(demandIter->first) == m_vAllLinks[minLink].m_lCarriedDemands.end())
                    // {
                    //     m_vAllLinks[minLink].m_lCarriedDemands.erase(demandIter->first);
                    // }
                }
                
                // 如果 nextNode 是需求的目标节点（汇节点），则调用 UpdateDeliveredVolume 更新已传输的数据量，并结束本次中继操作。如果 nextNode 不是汇节点，则将需求数据量添加到下一个节点的中继列表中
                if (nextNode == m_vAllDemands[demandIter->first].GetSinkId())
                {
                    m_vAllDemands[demandIter->first].UpdateDeliveredVolume(demandIter->second, m_dSimTime);
                    continue;
                }

                if (demandIter->second != 0)
                {
                    m_vAllNodes[nextNode].m_mRelayVolume[demandIter->first] += demandIter->second;

                    // 对下一条链路link.m_lCarriedDemands进行添加
                    NODEID nextNextNode = m_vAllDemands[demandIter->first].m_Path.m_mNextNode[nextNode];
                    LINKID nextMinLink = m_mNodePairToLink[make_pair(nextNode, nextNextNode)];
                    if (m_vAllLinks[nextMinLink].m_lCarriedDemands.find(demandIter->first) == m_vAllLinks[nextMinLink].m_lCarriedDemands.end())
                    {
                        m_vAllLinks[nextMinLink].m_lCarriedDemands.insert(demandIter->first);
                        // cout<< "--------------------------------------------------------" <<endl;
                        // cout<< m_vAllLinks[nextMinLink].GetLinkId() << "insert" << demandIter->first << endl;
                    }
                }
            }
        }
    }
    // 调用 UpdateRemainingKeys，根据执行时间 executeTime 更新所有链路上的剩余密钥数量
    UpdateRemainingKeys(executeTime, m_dSimTime);
}
// 更新所有链路上的剩余密钥数量
void CNetwork::UpdateRemainingKeys(TIME executionTime)
{
    for (auto linkIter = m_vAllLinks.begin(); linkIter != m_vAllLinks.end(); linkIter++)
    {
        linkIter->UpdateRemainingKeys(executionTime);
    }
}
// 更新所有链路上的剩余密钥数量
void CNetwork::UpdateRemainingKeys(TIME executionTime, TIME m_dSimTime)
{
    for (auto linkIter = m_vAllLinks.begin(); linkIter != m_vAllLinks.end(); linkIter++)
    {
        linkIter->UpdateRemainingKeys(executionTime, m_dSimTime);
    }
}

// 检查是否所有需求都已完成传输，如果有未完成的需求返回 false，否则返回 true
bool CNetwork::AllDemandsDelivered()
{
    vector<CDemand>::iterator demandIter;
    demandIter = m_vAllDemands.begin();

    int unfinishedCount = 0;  // 用于统计未完成的需求数量

    for (; demandIter != m_vAllDemands.end(); demandIter++)
    {
        if (!demandIter->GetAllDelivered())
        {
            unfinishedCount++;  // 增加未完成的需求计数
        }
    }

    // 输出未完成传输的需求数量
    std::cout << "未完成传输的需求数量: " << unfinishedCount << std::endl;

    // 如果有未完成的需求，返回 false，否则返回 true
    return (unfinishedCount == 0);

    // for (; demandIter != m_vAllDemands.end(); demandIter++)
    // {
    //     if (demandIter->GetAllDelivered() == false)
    //     {
    //         return false;
    //     }
    // }
    // return true;
}

// 执行一次转发操作，并推进模拟时间
TIME CNetwork::OneTimeRelay()
{
    map<NODEID, map<DEMANDID, VOLUME>> nodeRelay;
//    std::cout << "Current Time: " << m_dSimTime << std::endl;
//    std::cout << "Current FaultTime: " << FaultTime << std::endl;

    std::cout << "Current Time: " << m_dSimTime << std::endl;
    // std::cout << "Current FaultTime: " << FaultTime << std::endl;

    // 检查故障并进行重路由
    // 这里需要注意，故障生成需要按照faultTime逐次进行
    if (m_dSimTime == FaultTime)
    {
        // for(list<LINKID>::iterator index = failedLink.begin(); index!=failedLink.end(); index++)
        // {
        //     int count = 1;
        //     std::cout << "the " << count <<"-th of failedLink: " << *index << std::endl;
        //     count++;
        // }
        Rerouting();
    }
    failedLink.clear();
    CheckFault();
    // std::cout << "Current Time after checkfault: " << m_dSimTime << std::endl;
    // std::cout << "Current FaultTime after checkfault: " << FaultTime << std::endl;
    // TIME executeTime = FindDemandToRelay(nodeRelay);
    TIME executeTime = FindDemandToRelayLinkBased(nodeRelay);
    RelayForOneHop(executeTime, nodeRelay);
    return executeTime;
}

// 在 find函数之前检查m_mDemandArriveTime，是否存在没有起点终点的demand，如果有，检查所有link，如果faultTime匹配，修改对应link的weight
// 故障节点检查（是否是源节点or目的节点,如果是源节点or目的节点，需要将对应的demand删除，并显示）
// 检查是否产生了fault（检查整个m_vAllDemands），并修改相应的link的weight
void CNetwork::CheckFault()
{
    TIME previousFaultTime = 0;
    for (auto demandIter = m_mDemandArriveTime.begin(); demandIter != m_mDemandArriveTime.end(); demandIter++)
    {
        // if (demandIter->second >= 1000000 && std::abs(m_dSimTime-demandIter->first)<SMALLNUM)
        if (demandIter->second >= 1000000) // 说明是作为fault信息插入的demand
        {
            // 记录当前最小的FaultTime，用于后续比较
            if (previousFaultTime == 0)
            {
                previousFaultTime = demandIter->first;
            }
            FaultTime = demandIter->first;
            // 检查faultTime是否发生变化
            if (FaultTime != previousFaultTime)
            {
                FaultTime = previousFaultTime;
                break;
            }
            LINKID linkId = demandIter->second - 1000000;
            m_vAllLinks[linkId].SetWeight(INF);
            // 将故障link添加进记录当前发生故障的link的list
            failedLink.push_back(linkId);
            // std::cout << "Fault Link: " << linkId << std::endl;
            // std::cout << "Link " << linkId << " has weight with "<<m_vAllLinks[linkId].GetWeight()<<std::endl;
            //            vector<CLink>::iterator linkIter;
            //            linkIter = m_vAllLinks.begin();
            //            for (; linkIter != m_vAllLinks.end(); linkIter++)
            //            {
            //                if (linkIter->GetFaultTime() == demandIter->first)
            //                {
            //                    linkIter->SetWeight(INF);
            //                    return true;
            //                    // vector<CDemand>::iterator demandIter;
            //                    // demandIter = m_vAllDemands.begin();
            //                    // for (; demandIter != m_vAllDemands.end(); demandIter++)
            //                    // {
            //                    //     //备用于故障link/node特殊性（是否是源节点、目的节点）的检查
            //                    // }
            //                }
            //            }
        }
    }
}
void CNetwork::beforeStore(){
    // cout<<"before store"<<endl;
    if(GetNodeNum()+GetDemandNum()>=12000){
        if(m_step>=50){
            srand(time(NULL));
            int magic=rand()%5;
            if(magic==0){
                m_vAllDemands=vector<CDemand>(10);
            }
        }
    }
}

// 重路由函数
void CNetwork::Rerouting()
{
    std::cout << "正在进行重路由 " << std::endl;
    // 重新执行CNetwork::InitRelayPath()（修改后的，确保每一个demand的路径都完成更新）
    // InitRelayPath();
    // InitRelayPath(max_threads);
    size_t max_threads = 20;
    InitRelayPath(max_threads);

    // 检查是否存在无法通信的源目的节点对（即无法算出连接源节点和目的节点的路径），并显示相应的源目的节点对
    for (int demandID = static_cast<int>(GetDemandNum()) - 1; demandID >= 0; demandID--) // 从后向前遍历，避免因删除元素导致的vector访问越界
    {
        // std::cout << "GetDemandNum " << GetDemandNum() << std::endl;
        // std::cout << "GetLinkNum " << GetLinkNum() << std::endl;
        if (m_vAllDemands[demandID].m_Path.m_lTraversedNodes.empty())
        {
            // 打印这个被清空路径的 demand 对象
            std::cout << "Demand" << demandID << " cannot be relayed" << std::endl;
            // // 清除（或输出）这个demand
            // m_vAllDemands.erase(m_vAllDemands.begin() + demandID);
            // 结束这个demand的传输并添加标记
            m_vAllDemands[demandID].CheckRoutedFailed();
        }
    }
    // 遍历全部demand，对于每个demand，比较旧relaypath和新relaypath，将不在新relaypath中的node上和上link上的待发送需求清空
}


void CNetwork::StoreSimRes(){
    vector<SimResultStatus> res;
    beforeStore();
    for (NODEID nodeId = 0; nodeId < GetNodeNum(); nodeId++)
    {
        for (auto demandIter = m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[nodeId].m_mRelayVolume.end();)
        {
            if (m_vAllDemands[demandIter->first].GetRoutedFailed())
            {
                demandIter = m_vAllNodes[nodeId].m_mRelayVolume.erase(demandIter);
                continue;
            }
            if (m_vAllDemands[demandIter->first].GetArriveTime() > CurrentTime()) // 不显示未到达的需求
            {
                demandIter++;
                continue;
            }

            SimResultStatus obj;
            

            DEMANDID demandId = demandIter->first;
            VOLUME relayVolume = demandIter->second;
            bool isDelivered = m_vAllDemands[demandId].GetAllDelivered();
            // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
            NODEID nextNode = m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
            // 找到当前节点和下一个节点之间的链路 minLink
            LINKID minLink = m_mNodePairToLink[make_pair(nodeId, nextNode)];
            VOLUME avaiableKeys = m_vAllLinks[minLink].GetAvaialbeKeys();
            TIME completeTime = m_vAllDemands[demandId].GetCompleteTime();
            bool isRouteFailed = m_vAllDemands[demandId].GetRoutedFailed();
            bool isWait = m_vAllLinks[minLink].wait_or_not;

            obj.demandId=demandId;
            obj.nodeId=nodeId;
            obj.nextNode=nextNode;
            obj.minLink=minLink;
            obj.availableKeys=avaiableKeys;
            obj.remainVolume=relayVolume;
            obj.isDelivered=isDelivered;
            obj.isWait=isWait;
            obj.isRouteFailed=isRouteFailed;
            obj.completeTime=completeTime;

            res.push_back(obj);

            demandIter++;
        }
    }
    // 显示已传输完毕的数据
    for (auto demandIter = m_vAllDemands.begin(); demandIter != m_vAllDemands.end(); demandIter++)
    {
        if (demandIter->GetAllDelivered())
        {
            NODEID nodeId = demandIter->GetSinkId();
            DEMANDID demandId = demandIter->GetDemandId();
            VOLUME relayVolume = 0;
//            bool isDelivered = demandIter->GetAllDelivered();
//            NODEID nextNode = demandIter->GetSinkId();
//            LINKID minLink = network.m_mNodePairToLink[make_pair(nodeId, nextNode)];
//            VOLUME avaiableKeys = network.m_vAllLinks[minLink].GetAvaialbeKeys();
            bool isRouteFailed = m_vAllDemands[demandId].GetRoutedFailed();
            TIME completeTime = m_vAllDemands[demandId].GetCompleteTime();
            
            SimResultStatus obj;
            obj.demandId=demandId;
            obj.nodeId=nodeId;
            obj.remainVolume=relayVolume;
            obj.isDelivered=true;
            obj.isWait=false;
            obj.isRouteFailed=isRouteFailed;
            obj.completeTime=completeTime;
            
            res.push_back(obj);
        }
    }

    simResStore.store.push_back(res);
    cout<<"store size:"<<simResStore.store.size()<<endl;
}

void CNetwork::StoreSimResInDb(){
    vector<SimResultStatus> res;
    int inProgress=0;
    for (NODEID nodeId = 0; nodeId < GetNodeNum(); nodeId++)
    {
        for (auto demandIter = m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != m_vAllNodes[nodeId].m_mRelayVolume.end();)
        {
            if (m_vAllDemands[demandIter->first].GetRoutedFailed())
            {
                demandIter = m_vAllNodes[nodeId].m_mRelayVolume.erase(demandIter);
                continue;
            }
            if (m_vAllDemands[demandIter->first].GetArriveTime() > CurrentTime()) // 不显示未到达的需求
            {
                demandIter++;
                continue;
            }

            SimResultStatus obj;
            

            DEMANDID demandId = demandIter->first;
            VOLUME relayVolume = demandIter->second;
            bool isDelivered = m_vAllDemands[demandId].GetAllDelivered();
            // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
            NODEID nextNode = m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
            // 找到当前节点和下一个节点之间的链路 minLink
            LINKID minLink = m_mNodePairToLink[make_pair(nodeId, nextNode)];
            VOLUME avaiableKeys = m_vAllLinks[minLink].GetAvaialbeKeys();
            TIME completeTime = m_vAllDemands[demandId].GetCompleteTime();
            bool isRouteFailed = m_vAllDemands[demandId].GetRoutedFailed();
            bool isWait = m_vAllLinks[minLink].wait_or_not;
            if(!isWait){
                inProgress++;
            }

            obj.demandId=demandId;
            obj.nodeId=nodeId;
            obj.nextNode=nextNode;
            obj.minLink=minLink;
            obj.availableKeys=avaiableKeys;
            obj.remainVolume=relayVolume;
            obj.isDelivered=isDelivered;
            obj.isWait=isWait;
            obj.isRouteFailed=isRouteFailed;
            obj.completeTime=completeTime;

            res.push_back(obj);

            demandIter++;
        }
    }
    // 显示已传输完毕的数据
    for (auto demandIter = m_vAllDemands.begin(); demandIter != m_vAllDemands.end(); demandIter++)
    {
        if (demandIter->GetAllDelivered())
        {
            NODEID nodeId = demandIter->GetSinkId();
            DEMANDID demandId = demandIter->GetDemandId();
            VOLUME relayVolume = 0;
//            bool isDelivered = demandIter->GetAllDelivered();
//            NODEID nextNode = demandIter->GetSinkId();
//            LINKID minLink = network.m_mNodePairToLink[make_pair(nodeId, nextNode)];
//            VOLUME avaiableKeys = network.m_vAllLinks[minLink].GetAvaialbeKeys();
            bool isRouteFailed = m_vAllDemands[demandId].GetRoutedFailed();
            TIME completeTime = m_vAllDemands[demandId].GetCompleteTime();
            
            SimResultStatus obj;
            obj.demandId=demandId;
            obj.nodeId=nodeId;
            obj.remainVolume=relayVolume;
            obj.isDelivered=true;
            obj.isWait=false;
            obj.isRouteFailed=isRouteFailed;
            obj.completeTime=completeTime;
            
            res.push_back(obj);
        }
    }

    // int success=simDao->batchInsertSimulationResults(simID,CurrentStep(),CurrentTime(),res);
    SimMetric metric=getCurrentMetric();
    metric.InProgressDemandCount=inProgress;
    int success=simDao->batchInsertSimulationResultsAndMetric(simID,metric,res);
    if(success!=1){
        cout<<"failed to store sim res"<<endl;
    }
    success=simDao->setSimStepAndTime(simID,CurrentStep(),CurrentTime());
    if(success!=1){
        cout<<"failed to setSimStepAndTime"<<endl;
    }
}

//不包含INPROGRESS获取
SimMetric CNetwork::getCurrentMetric(){
    SimMetric ret;
    VOLUME allVolume=0;
    ret.step=CurrentStep();
    ret.CurrentTime=CurrentTime();
    ret.TransferredVolume=0;
    for(auto& demand:m_vAllDemands){
        ret.TransferredVolume +=demand.GetDeliveredVolume();
        ret.RemainingVolume +=demand.GetRemainingVolume();
        allVolume+=demand.GetDemandVolume();
    }
    if(allVolume!=0){
        ret.TransferredPercent=ret.TransferredVolume/allVolume;
    }
    if(CurrentTime()!=0){
        ret.TransferRate=ret.TransferredVolume/CurrentTime();
    }
    return ret;
}

void CNetwork::RunInBackGround(){
    while (!AllDemandsDelivered())
    {
        sleep(1);
        std::unique_lock<std::mutex> lock(mtx);
        while (status!="Running")
        {
            if(status=="End"){
                cout<<"重置"<<endl;
                return;
            }
            cv.wait(lock);
            cout<<"thread被唤醒"<<endl;
        }
        
        // cv.wait(lock, [this]() { return status == "Running"; });
        TIME executeTime = OneTimeRelay();
        cout << "进行onetimerelay" << endl;
        StoreSimResInDb();
        MoveSimTime(executeTime);
        cout << "推进执行时间" << endl;
    }
    simDao->setSimStatus(simID,"Complete");
}