﻿#include "KeyManager.h"
#include <cassert>

CKeyManager::CKeyManager(void)
{
}


CKeyManager::~CKeyManager(void)
{
}

CKeyManager::CKeyManager(const CKeyManager& keyManager)
{
    m_uiKeyManagerId = keyManager.m_uiKeyManagerId;
    m_uiAssociatedNode = keyManager.m_uiAssociatedNode;
    m_uiPairedNode = keyManager.m_uiPairedNode;
    m_uiAssociatedLink = keyManager.m_uiAssociatedLink;
    m_dAvailableKeys = keyManager.m_dAvailableKeys;
    m_dKeyRate = keyManager.m_dKeyRate;
}

void CKeyManager::operator=(const CKeyManager& keyManager)
{
    m_uiKeyManagerId = keyManager.m_uiKeyManagerId;
    m_uiAssociatedNode = keyManager.m_uiAssociatedNode;
    m_uiPairedNode = keyManager.m_uiPairedNode;
    m_uiAssociatedLink = keyManager.m_uiAssociatedLink;
    m_dAvailableKeys = keyManager.m_dAvailableKeys;
    m_dKeyRate = keyManager.m_dKeyRate;
}

void CKeyManager::SetKeyManagerId(KEYID keyId)
{
    m_uiKeyManagerId = keyId;
}
KEYID CKeyManager::GetKeyManagerId()
{
    return m_uiKeyManagerId;
}

void CKeyManager::SetAssociatedNode(NODEID associatedNode)
{
    m_uiAssociatedNode = associatedNode;
}
NODEID CKeyManager::GetAssociatedNode()
{
    return m_uiAssociatedNode;
}

void CKeyManager::SetPairedNode(NODEID pairedNode)
{
    m_uiPairedNode = pairedNode;
}
NODEID CKeyManager::GetPairedNode()
{
    return m_uiPairedNode;
}

void CKeyManager::SetAvailableKeys(VOLUME keys)
{
    m_dAvailableKeys = keys;
}
VOLUME CKeyManager::GetAvailableKeys()
{
    return m_dAvailableKeys;
}
void CKeyManager::ConsumeKeys(VOLUME keys)
{
    m_dAvailableKeys -= keys;
    // if (m_dAvailableKeys < keys)
    // {
    //     throw std::runtime_error("Not enough available keys to consume!");
    //     //m_dAvailableKeys = 12345;
    // }
    // else
    // {
    //     m_dAvailableKeys -= keys;
    // }
}
void CKeyManager::CollectKeys(VOLUME keys)
{
    m_dAvailableKeys += keys;
}
void CKeyManager::SetAssociatedLink(LINKID linkId)
{
    m_uiAssociatedLink = linkId;
}
LINKID CKeyManager::GetAssociateLink()
{
    return m_uiAssociatedLink;
}

void CKeyManager::SetKeyRate(RATE keyRate)
{
    m_dKeyRate = keyRate;
}
RATE CKeyManager::GetKeyRate()
{
    return m_dKeyRate;
}

void CKeyManager::InvalidateKeys(double failureProbability, std::default_random_engine& generator)
{
    // 使用伯努利分布决定失效数量
    std::bernoulli_distribution distribution(failureProbability);
    //int invalidatedKeys = 0;

    for (int i = 0; i < static_cast<int>(m_dAvailableKeys); ++i) { // 转换为 int 以避免警告
        if (distribution(generator)) {
            --m_dAvailableKeys;
           // ++invalidatedKeys;
        }
    }

    // std::cout << "Invalidated " << invalidatedKeys << " keys. Total: " << m_dAvailableKeys << std::endl;
}
