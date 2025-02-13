#pragma once
#include "stdafx.h"
#include <mutex>

class CNode
{
public:
    CNode(void);
    ~CNode(void);
    CNode(const CNode& Node);
    void operator=(const CNode& Node);

private:
    NODEID m_uiNodeID;	// 节点的唯一标识符（ID）
    
public:
    list<NODEID> m_lAdjNodes;	// 与当前节点相邻的其他节点的ID
    list<NODEID> m_lAdjLinks;	// 与当前节点相连的链路的ID    未使用

    // 添加互斥锁保护共享数据
    std::mutex m_mutex; // 保护节点数据的互斥锁
    std::unordered_map<DEMANDID, VOLUME> m_lCarriedDemands; // 使用 unordered_map 存储需求ID和剩余数据量


    unordered_map<DEMANDID, VOLUME> m_mRelayVolume;
    
    // pair<unordered_map<DEMANDID, VOLUME>, bool> m_mRelayVolume;	// 用于记录需要通过此节点转发的数据量。键是DEMANDID（表示需求ID），值是VOLUME（表示数据量）。
    // struct Node
    // {
    //     unordered_map<DEMANDID, VOLUME>
    //     bool 
    // };
    
    
public:
    void SetNodeId(NODEID nodeId);
    NODEID GetNodeId();

};

