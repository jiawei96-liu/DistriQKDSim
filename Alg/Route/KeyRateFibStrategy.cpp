#include "KeyRateFibStrategy.h"
#include "FibonacciHeap.h"
#include "Alg/Network.h"

using namespace route;

bool KeyRateFibStrategy::Route(NODEID sourceId, NODEID sinkId, 
                              std::list<NODEID>& nodeList, 
                              std::list<LINKID>& linkList) {
    // ====================== 用户可配置参数 ======================
    // 1. 节点数量获取方式（可根据实际网络结构修改）
    UINT NodeNum = static_cast<UINT>(net->m_vAllNodes.size());
    
    // 2. 前驱节点记录容器（可替换为其他数据结构）
    vector<NODEID> preNode(NodeNum, sourceId);
    
    // 3. 节点访问状态记录（可替换为其他标记方式）
    vector<bool> visited(NodeNum, false);
    
    // 4. 优先队列选择（可替换为std::priority_queue或其他堆实现）
    using PQNode = pair<RATE, NODEID>;
    vector<FibHeap<PQNode>::FibNode*> ptr(NodeNum, nullptr);
    FibHeap<PQNode> pq;
    
    // ====================== 算法核心逻辑 ======================
    // 初始化起点
    auto* sourceNode = pq.push({0, sourceId});
    ptr[sourceId] = sourceNode;

    while (!pq.empty()) {
        // 获取当前最小距离节点
        auto [curDistVal, curNode] = pq.top();
        pq.pop();
        
        // 节点访问检查（可修改为其他跳过条件）
        if (visited[curNode]) continue;
        visited[curNode] = true;
        
        // 终止条件判断（可修改为目标检查逻辑）
        if (curNode == sinkId) break;
        
        // ============== 邻接节点遍历（关键可修改区域） ==============
        for (auto adjNodeIter = net->m_vAllNodes[curNode].m_lAdjNodes.begin();
             adjNodeIter != net->m_vAllNodes[curNode].m_lAdjNodes.end();
             adjNodeIter++) {
            NODEID adjNode = *adjNodeIter;
            
            // 邻接节点过滤条件（可添加额外过滤逻辑）
            if (visited[adjNode]) continue;
            
            // ============== 链路选择逻辑（关键可修改区域） ==============
            // 获取当前节点与邻接节点间的链路
            LINKID midLink = net->m_mNodePairToLink[make_pair(curNode, adjNode)];
            
            // ============== 成本计算逻辑（关键可修改区域） ==============
            // 当前成本：链路密钥生成速率
            // 可修改为其他成本计算方式（如中继延迟、带宽、跳数等）
            RATE newDist = ptr[curNode]->key.first + net->m_vAllLinks[midLink].GetQKDRate();

            // // 将中继延迟作为成本考量
            // RATE newDist = ptr[curNode]->key.first + net->m_vAllLinks[midLink].GetLinkDelay();
            
            // ============== 距离更新逻辑（关键可修改区域） ==============
            // 松弛操作：检查是否需要更新距离
            if (ptr[adjNode] == nullptr || newDist < ptr[adjNode]->key.first) {
                preNode[adjNode] = curNode;  // 更新前驱节点
                
                if (ptr[adjNode] == nullptr) {
                    // 新节点加入优先队列
                    auto* adjNodePtr = pq.push({newDist, adjNode});
                    ptr[adjNode] = adjNodePtr;
                } else {
                    // 更新已有节点的距离
                    pq.decrease_key(ptr[adjNode], {newDist, adjNode});
                }
            }
        }
    }
    
    // ====================== 路径重构逻辑 ======================
    // 检查路径是否存在（可修改为其他路径存在性检查）
    if (!visited[sinkId]) return false;
    
    // 从终点回溯构建路径（可修改路径存储方式）
    NODEID curNode = sinkId;
    while (curNode != sourceId) {
        nodeList.push_front(curNode);
        NODEID pre = preNode[curNode];
        
        // 获取节点间链路（可修改为其他链路获取方式）
        LINKID midLink = net->m_mNodePairToLink[make_pair(pre, curNode)];
        linkList.push_front(midLink);
        
        curNode = pre;
    }
    nodeList.push_front(sourceId);
    
    return true;
}