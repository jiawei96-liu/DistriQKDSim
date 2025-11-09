#include "Link.h"
#include <algorithm>

// CLink::CLink(void)
// {
//     wait_or_not = true;
// }



CLink::CLink(void)
    : m_uiLinkId(0), m_uiSourceId(0), m_uiSinkId(0), m_dQKDRate(0), m_dDelay(0), m_dBandwidth(0), m_dFaultTime(0), m_iSubdomainId(0), m_dWeight(0)
{
    wait_or_not = true;
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
    m_lCarriedDemands = link.m_lCarriedDemands;
    m_dFaultTime = link.m_dFaultTime;
    m_iSubdomainId = link.m_iSubdomainId;
    m_dWeight = link.m_dWeight;
    wait_or_not = link.wait_or_not;
}

void CLink::operator=(const CLink& link)
{
    m_uiLinkId = link.m_uiLinkId;
    m_uiSourceId = link.m_uiSourceId;
    m_uiSinkId = link.m_uiSinkId;
    m_iSubdomainId = link.m_iSubdomainId;
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

// reservation APIs
VOLUME CLink::GetAvailableForReservation()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    VOLUME avail = m_KeyManager.GetAvailableKeys();
    if (avail <= m_totalReserved) return 0;
    return avail - m_totalReserved;
}

bool CLink::TryReserve(DEMANDID demandid, VOLUME vol, TIME arriveTime)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    VOLUME avail = m_KeyManager.GetAvailableKeys();
    VOLUME availForRes = 0;
    if (avail > m_totalReserved) availForRes = avail - m_totalReserved;
    if (availForRes >= vol) {
        m_reservations[demandid] += vol;
        m_totalReserved += vol;
        return true;
    }
    return false;
}

void CLink::CommitReservation(DEMANDID demandid)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_reservations.find(demandid);
    if (it == m_reservations.end()) return;
    VOLUME vol = it->second;
    // consume actual keys from key manager
    m_KeyManager.ConsumeKeys(vol);
    // adjust reserved accounting and erase
    if (m_totalReserved >= vol) m_totalReserved -= vol; else m_totalReserved = 0;
    m_reservations.erase(it);
}

void CLink::ReleaseReservation(DEMANDID demandid)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_reservations.find(demandid);
    if (it == m_reservations.end()) return;
    VOLUME vol = it->second;
    if (m_totalReserved >= vol) m_totalReserved -= vol; else m_totalReserved = 0;
    m_reservations.erase(it);
}

void CLink::AddToWaitingQueue(DEMANDID demandid, VOLUME vol, TIME arriveTime, NODEID originNode)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    WaitingRequest req{demandid, vol, arriveTime, originNode};
    m_waitingQueue.push_back(req);
}


//---------------------------------添加部分---------------------------------
void CLink::SetClassicalBandwidth(WEIGHT weight)
{
    m_dClassicalBandwidth = weight * m_dBandwidth; // 使用 weight 参数作为比例因子
}


RATE CLink::GetClassicalBandwidth()
{
    return m_dClassicalBandwidth;
}


void CLink::SetNegotiationBandwidth()
{
    m_dNegotiationBandwidth = m_dBandwidth - m_dClassicalBandwidth;
}

RATE CLink::GetNegotiationBandwidth()
{
    return m_dNegotiationBandwidth;
}

void CLink::SetKeyRateCoefficient()
{
    m_KeyManager.SetKeyRate(KEY_RATE_TO_NEGOTIATION_RATIO * m_dNegotiationBandwidth);
}
//-------------------------------------------------------------------------


