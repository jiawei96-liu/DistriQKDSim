#pragma once
#include "stdafx.h"
#include "Network.h"
class CInputOutput
{
public:
    CInputOutput(void);
    ~CInputOutput(void);
    CInputOutput(CNetwork* Network);

    CNetwork* m_pNetwork;	// 指向CNetwork对象的指针，用于在CInputOutput类中操作和修改网络
private:
    void InitNodes(UINT nodeNum);	// 根据指定的节点数量初始化网络中的节点，将节点添加到网络的节点列表中

public:
    void InputNetworkInfo(string fileName);	// 从指定的文件中读取网络的拓扑信息（例如节点和链路的配置），并将这些信息加载到网络结构中
    void InputDemandInfo(string fileName);	// 从指定的文件中读取需求信息（例如每个需求的源节点、目的节点、数据量等），并将这些信息加载到网络的需求列表中

};

