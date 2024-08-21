#pragma once
class CKeyManager
{
public:
	CKeyManager(void);
	~CKeyManager(void);
	CKeyManager(const CKeyManager& keyManager);
	void operator=(const CKeyManager& keyManager);

private:
	KEYID m_uiKeyManagerId;	// ��Կ��������Ψһ��ʶ��
	NODEID m_uiAssociatedNode;	// �����Կ�����������Ľڵ�ID
	NODEID m_uiPairedNode;	// ��֮��Ե���һ���ڵ��ID
	LINKID m_uiAssociatedLink;	// ����Կ�������������·ID
	VOLUME m_dAvailableKeys;	// ��ǰ���õ���Կ����
	RATE m_dKeyRate;	// ��Կ��������

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


	void ConsumeKeys(VOLUME keys);	// ����ָ����������Կ
	void CollectKeys(VOLUME keys);	// ����ָ����������Կ��ͨ�����ڸ���ʱ�������µ���Կ��
};

