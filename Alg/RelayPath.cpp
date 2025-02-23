﻿#include "RelayPath.h"

CRelayPath::CRelayPath(void)
{
}


CRelayPath::~CRelayPath(void)
{
}

CRelayPath::CRelayPath(const CRelayPath& path)
{
    m_uiSourceId = path.m_uiSourceId;
    m_uiSinkId = path.m_uiSinkId;
    m_uiAssociateDemand = path.m_uiAssociateDemand;
    m_lTraversedNodes = path.m_lTraversedNodes;
    m_lTraversedLinks = path.m_lTraversedLinks;
    m_mNextNode = path.m_mNextNode;
}

void CRelayPath::operator=(const CRelayPath& path)
{
    m_uiSourceId = path.m_uiSourceId;
    m_uiSinkId = path.m_uiSinkId;
    m_uiAssociateDemand = path.m_uiAssociateDemand;
    m_lTraversedNodes = path.m_lTraversedNodes;
    m_lTraversedLinks = path.m_lTraversedLinks;
    m_mNextNode = path.m_mNextNode;
}

void CRelayPath::SetSourceId(NODEID SourceId)
{
    m_uiSourceId = SourceId;
}

NODEID CRelayPath::GetSourceId()
{
    return m_uiSourceId;
}

void CRelayPath::SetSinkId(NODEID sinkId)
{
    m_uiSinkId = sinkId;
}

NODEID CRelayPath::GetSinkId()
{
    return m_uiSinkId;
}

void CRelayPath::SetAssociateDemand(DEMANDID demandId)
{
    m_uiAssociateDemand = demandId;
}

NODEID CRelayPath::GetAssocaiteDemand()
{
    return m_uiAssociateDemand;
}

// 实现 Clear 方法
void CRelayPath::Clear()
{
    m_lTraversedNodes.clear();
    m_lTraversedLinks.clear();
    m_mNextNode.clear();
}
