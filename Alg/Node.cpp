#include "Node.h"

// CNode::CNode(void)
// {
// }



CNode::CNode(void)
    : m_uiNodeID(0), m_iSubdomainId(0), m_bIsGateway(false)
{
}



CNode::CNode(const CNode& node)
{
    m_uiNodeID = node.m_uiNodeID;
    m_iSubdomainId = node.m_iSubdomainId;
    m_bIsGateway = node.m_bIsGateway;
    m_lAdjNodes = node.m_lAdjNodes;
    m_lAdjLinks = node.m_lAdjLinks;
}


void CNode::operator=(const CNode& node)
{
    m_uiNodeID = node.m_uiNodeID;
    m_iSubdomainId = node.m_iSubdomainId;
    m_bIsGateway = node.m_bIsGateway;
    m_lAdjNodes = node.m_lAdjNodes;
    m_lAdjLinks = node.m_lAdjLinks;
}

void CNode::SetNodeId(NODEID nodeId)
{
    m_uiNodeID = nodeId;
}

NODEID CNode::GetNodeId() const
{
    return m_uiNodeID;
}
