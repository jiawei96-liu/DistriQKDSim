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
    NODEID m_uiNodeID; // 节点的唯一标识符（ID）
    int m_iSubdomainId = 0; // 子域标识，默认0
    bool m_bIsGateway = false; // 是否为网关节点，默认false

public:
    list<NODEID> m_lAdjNodes; // 与当前节点相邻的其他节点的ID
    list<NODEID> m_lAdjLinks; // 与当前节点相连的链路的ID    未使用

    // 添加互斥锁保护共享数据
    std::mutex m_mutex; // 保护节点数据的互斥锁
    std::unordered_map<DEMANDID, VOLUME> m_lCarriedDemands; // 使用 unordered_map 存储需求ID和剩余数据量

    unordered_map<DEMANDID, VOLUME> m_mRelayVolume;

    // 子域与网关相关接口
    void SetSubdomainId(int subdomainId) { m_iSubdomainId = subdomainId; }
    int GetSubdomainId() const { return m_iSubdomainId; }
    void SetIsGateway(bool isGateway) { m_bIsGateway = isGateway; }
    bool IsGateway() const { return m_bIsGateway; }

public:
    void SetNodeId(NODEID nodeId);
    NODEID GetNodeId();

};

