#pragma once
#include "stdafx.h"
class CRelayPath
{
public:
    CRelayPath(void);
    ~CRelayPath(void);
    CRelayPath(const CRelayPath& path);
    void operator=(const CRelayPath& path);

private:
    NODEID m_uiSourceId;	// 路径的源节点ID
    NODEID m_uiSinkId;	// 路径的目的节点ID（汇节点）
    DEMANDID m_uiAssociateDemand;	// 与此路径关联的需求ID

public:
    list<NODEID> m_lTraversedNodes;	// 一个节点ID的列表，表示路径上依次经过的节点
    list<LINKID> m_lTraversedLinks;	// 一个链路ID的列表，表示路径上依次经过的链路
    unordered_map<NODEID, NODEID> m_mNextNode;	// 一个节点ID到下一个节点ID的映射，用于快速查找路径中某节点的下一个节点

public:
    void SetSourceId(NODEID SourceId);
    NODEID GetSourceId();

    void SetSinkId(NODEID sinkId);
    NODEID GetSinkId();

    void SetAssociateDemand(DEMANDID demandId);
    DEMANDID GetAssocaiteDemand();

    // 添加清空路径的方法
    void Clear();
};

