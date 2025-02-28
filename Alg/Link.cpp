#include "Link.h"
#include <algorithm>

CLink::CLink(void)
{
    wait_or_not = true;
}


CLink::~CLink(void)
{
}

CLink::CLink(const CLink& link)
{
    m_uiLinkId = link.m_uiLinkId;
    m_uiSourceId = link.m_uiSourceId;
    m_uiSinkId = link.m_uiSinkId;
    m_dQKDRate = link.m_dQKDRate;
    m_dDelay = link.m_dDelay;
    m_KeyManager = link.m_KeyManager;
    m_dBandwidth = link.m_dBandwidth;
    
    // 为啥给注释了
    m_lCarriedDemands = link.m_lCarriedDemands;
    
    
    m_dFaultTime = link.m_dFaultTime;

    m_dWeight = link.m_dWeight;

    
    wait_or_not = link.wait_or_not;
}

void CLink::operator=(const CLink& link)
{
    m_uiLinkId = link.m_uiLinkId;
    m_uiSourceId = link.m_uiSourceId;
    m_uiSinkId = link.m_uiSinkId;
    m_dQKDRate = link.m_dQKDRate;
    m_dDelay = link.m_dDelay;
    m_KeyManager = link.m_KeyManager;
    m_dBandwidth = link.m_dBandwidth;
    // 为啥给注释了
    m_lCarriedDemands = link.m_lCarriedDemands;
    
    m_dFaultTime = link.m_dFaultTime;

    m_dWeight = link.m_dWeight;
}

void CLink::SetLinkId(LINKID linkId)
{
    m_uiLinkId = linkId;
}

LINKID CLink::GetLinkId()
{
    return m_uiLinkId;
}

void CLink::SetSourceId(NODEID sourceId)
{
    m_uiSourceId = sourceId;
}

NODEID CLink::GetSourceId()
{
    return m_uiSourceId;
}

void CLink::SetSinkId(NODEID sinkId)
{
    m_uiSinkId = sinkId;
}

NODEID CLink::GetSinkId()
{
    return m_uiSinkId;
}

void CLink::SetQKDRate(RATE QKDRate)
{
    m_dQKDRate = QKDRate;
}

RATE CLink::GetQKDRate()
{
    return m_dQKDRate;
}

void CLink::SetLinkDelay(TIME delay)
{
    m_dDelay = delay;
}

TIME CLink::GetLinkDelay()
{
    return m_dDelay;
}

void CLink::SetBandwidth(RATE bandwidth)
{
    m_dBandwidth = bandwidth;
}

RATE CLink::GetBandwidth()
{
    return m_dBandwidth;
}

void CLink::SetWeight(WEIGHT linkWeight)
{
    m_dWeight = linkWeight;
}

WEIGHT CLink::GetWeight()
{
    return m_dWeight;
}


void CLink::SetFaultTime(TIME faultTime)
{
    m_dFaultTime = faultTime;
}

TIME CLink::GetFaultTime()
{
    return m_dFaultTime;
}

//密钥相关
void CLink::SetKeyManager(const CKeyManager& keyManager)
{
    m_KeyManager = keyManager;
}

void CLink::ConsumeKeys(VOLUME keys)
{
    m_KeyManager.ConsumeKeys(keys);
}
VOLUME CLink::GetAvaialbeKeys()
{
    return m_KeyManager.GetAvailableKeys();
}
void CLink::UpdateRemainingKeys(TIME executionTime)
{
    m_KeyManager.CollectKeys(executionTime*m_KeyManager.GetKeyRate());
}
// 修改后的 UpdateRemainingKeys 方法
void CLink::UpdateRemainingKeys(TIME executionTime, TIME m_dSimTime)
{
    // 获取当前的关键速率
    double m_dKeyRate = m_KeyManager.GetKeyRate();
//    cout<<"KeyRate:"<<m_dKeyRate<<endl;
    m_KeyManager.CollectKeys(m_dKeyRate * executionTime);
    // // 将当前仿真时间转换为种子
    // unsigned int seed = static_cast<unsigned int>(m_dSimTime);
    // std::default_random_engine generator(seed);

    // // 计算标准差并确保其为正值
    // double stddev = 0.1 * m_dKeyRate;
    // // stddev = std::max(stddev, 1e-9); // 使用 std::max 来确保最小标准差
    // stddev = max(stddev, 1e-9); // 使用 max 来确保最小标准差

    // // 创建一个正态分布器，以 m_dKeyRate 为均值，以经过验证的标准差为标准差
    // std::normal_distribution<double> distribution(m_dKeyRate, stddev);

    // // 生成一个随机值，表示新增加的密钥数量
    // double random_value = distribution(generator) * executionTime;

    // // 确保随机值不能小于零
    // if (random_value < 0.0) {
    //     random_value = 0.0;
    // }

    // // 使用计算出的随机值更新剩余的键
    // m_KeyManager.CollectKeys(random_value);

    // // 根据时间差和失效概率失效部分密钥
    // double failureProbabilityPerUnitTime = 0.1; // 示意性失效概率
    // for (TIME t = 0; t < executionTime; ++t) {
    //     m_KeyManager.InvalidateKeys(failureProbabilityPerUnitTime, generator);
    // }
}


