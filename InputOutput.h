#pragma once
#include "Network.h"
class CInputOutput
{
public:
	CInputOutput(void);
	~CInputOutput(void);
	CInputOutput(CNetwork* Network);

	CNetwork* m_pNetwork;	// ָ��CNetwork�����ָ�룬������CInputOutput���в������޸�����
private:
	void InitNodes(UINT nodeNum);	// ����ָ���Ľڵ�������ʼ�������еĽڵ㣬���ڵ���ӵ�����Ľڵ��б���

public:
	void InputNetworkInfo(string fileName);	// ��ָ�����ļ��ж�ȡ�����������Ϣ������ڵ����·�����ã���������Щ��Ϣ���ص�����ṹ��
	void InputDemandInfo(string fileName);	// ��ָ�����ļ��ж�ȡ������Ϣ������ÿ�������Դ�ڵ㡢Ŀ�Ľڵ㡢�������ȣ���������Щ��Ϣ���ص�����������б���

};

