#pragma once
#include <random>
#include "stdafx.h"
class CKeyManager
{
public:
    CKeyManager(void);
    ~CKeyManager(void);
    CKeyManager(const CKeyManager& keyManager);
    void operator=(const CKeyManager& keyManager);

private:
    KEYID m_uiKeyManagerId;	// 密钥管理器的唯一标识符
    NODEID m_uiAssociatedNode;	// 与此密钥管理器关联的节点ID
    NODEID m_uiPairedNode;	// 与之配对的另一个节点的ID
    LINKID m_uiAssociatedLink;	// 此密钥管理器管理的链路ID
    VOLUME m_dAvailableKeys;	// 当前可用的密钥数量
    RATE m_dKeyRate;	// 密钥生成速率

public:
    void SetKeyManagerId(KEYID keyId);
    KEYID GetKeyManagerId();

    void SetAssociatedNode(NODEID associatedNode);
    NODEID GetAssociatedNode();

    void SetPairedNode(NODEID pairedNode);
    NODEID GetPairedNode();

    void SetAvailableKeys(VOLUME keys);
    VOLUME GetAvailableKeys();

    void SetAssociatedLink(LINKID linkId);
    LINKID GetAssociateLink();

    void SetKeyRate(RATE keyRate);
    RATE GetKeyRate();


    void ConsumeKeys(VOLUME keys);	// 消耗指定数量的密钥
    void CollectKeys(VOLUME keys);	// 增加指定数量的密钥（通常用于根据时间生成新的密钥）

    void InvalidateKeys(double failureProbability, std::default_random_engine& generator);//密钥随着时间有一定概率失效
};

