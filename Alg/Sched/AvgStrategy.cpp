#include "AvgStrategy.h"
#include "Alg/Network.h"

using namespace sched;
TIME AvgStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
        // 创建一个随机数引擎
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(0, 1); // 定义一个分布，生成 0 或 1

    // 生成随机数
    int tempWait = dis(gen);

    // cout<<"进入平均密钥调度算法"<<endl;
    TIME executeTime = INF;                // 表示当前的最小执行时间
    // 遍历link
    auto& m_vAllLinks=net->m_vAllLinks;
    auto& m_dSimTime=net->m_dSimTime;
    auto& m_vAllDemands=net->m_vAllDemands;
    auto& m_vAllNodes=net->m_vAllNodes;
    for(auto &link : m_vAllLinks)
    {
        // 只遍历链路首或尾是该节点的链路
        if (link.GetSourceId() != nodeId && link.GetSinkId() != nodeId)
        {
            continue;
        }
        // cout<<"该节点存在链路"<<endl;
        // 跳过故障link和link上没需求的link
        // if (link.GetFaultTime() >= 0 && link.GetFaultTime() <= m_dSimTime || link.m_lCarriedDemands.empty())
        if (link.GetFaultTime() > 0 && link.GetFaultTime() <= m_dSimTime || link.m_lCarriedDemands.empty())
        {
            link.wait_or_not = true;
            continue;
        }
        
        VOLUME availableKeyVolume = link.GetAvaialbeKeys();
        RATE bandwidth = link.GetBandwidth();
        NODEID sourceid =  link.GetSourceId();
        NODEID sinkid =  link.GetSinkId();
        TIME tempTime = INF;
        int num_of_demand = 0;
        // 遍历link上的demand，得到一条链路上的执行时间
        for (auto &demandid : link.m_lCarriedDemands)
        {
            // cout<<demandid<<endl;
            if (m_vAllDemands[demandid].GetArriveTime() > m_dSimTime + SMALLNUM)
            {
                // this demand has not arrived yet
                continue;
            }
            // cout<<"该链路存在需求"<<endl;
            num_of_demand++;
            cout<< "平均密钥调度:"<< link.GetLinkId()<< "have demand" << demandid<<endl;
            NODEID nodeid;
            VOLUME relayVolume;
            auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
            // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
            if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
            {
                nodeid = sourceid;
                relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
            }
            else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
            {
                nodeid = sinkid;
                relayVolume = m_vAllNodes[nodeid].m_mRelayVolume[demandid];
            }
            // else
            // {
            //     throw 1;
            // }

            // 对一个demand，判断链路最小执行时间tempTime
            if (relayVolume / bandwidth < tempTime)
            {
                tempTime = relayVolume / bandwidth;
            }
            // cout << "relayVolume" << relayVolume<<endl;
            // cout << "bandwidth" << bandwidth<<endl;
            // cout << "tempTime" << tempTime<<endl;
        }
        // 没有可以传的需求
        if (num_of_demand == 0)
        {
            continue;
        }
        // 找到了该条链路上的最小执行时间tempTime，计算最小传输量，然后比较可用密钥量
        // VOLUME needVolume = tempTime * bandwidth * link.m_lCarriedDemands.size();
        VOLUME needVolume = tempTime * bandwidth * num_of_demand;
        // cout << "needVolume" << needVolume<<endl;
        // cout << "availableKeyVolume" << availableKeyVolume<<endl;
        // 如果可用密钥量足够，给每一个nodeid，赋值传同样的最小传输量
        if (needVolume <= availableKeyVolume)
        {
            link.wait_or_not = false;
            for (auto &demandid : link.m_lCarriedDemands)
            {
                NODEID nodeid;
                auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
                // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
                if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
                {
                    nodeid = sourceid;
                }
                else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
                {
                    nodeid = sinkid;
                }
                // else
                // {
                //     throw 1;
                // }
                // 对每一个可以传输的demand，给相应的nodeid传输最小传输量
                if (nodeid == nodeId)
                {
                    relayDemands[demandid] = tempTime * bandwidth;
                }
            }
        }
        else if (availableKeyVolume >= 50)
        {
            link.wait_or_not = false;
            for (auto &demandid : link.m_lCarriedDemands)
            {
                NODEID nodeid;
                auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
                // relayVolume就是找到的待传的数据，这个m_mRelayVolume在sourceid或sinkid上
                if (nextNode.count(sourceid) && nextNode[sourceid] == sinkid)
                {
                    nodeid = sourceid;
                }
                else if (nextNode.count(sinkid) && nextNode[sinkid] == sourceid)
                {
                    nodeid = sinkid;
                }
                // else
                // {
                //     throw 1;
                // }
                // 对每一个可以传输的demand，给相应的nodeid传输最小传输量
                if (nodeid == nodeId)
                {
                    // relayDemands[demandid] = availableKeyVolume / link.m_lCarriedDemands.size();
                    relayDemands[demandid] = availableKeyVolume / num_of_demand;
                }
            }
        }
        else
        {
            link.wait_or_not = true;
            tempTime = (needVolume - availableKeyVolume) / link.GetQKDRate();
        }
        // if (tempTime < 3)
        // {
        //     // if (tempWait == 1)
        //     // {
        //     //     link.wait_or_not = true;
        //     //     continue;
        //     // }
        //     link.wait_or_not = true;
        //     executeTime = 3;
        //     continue;
        // }
        if (tempTime < executeTime)
        {
            executeTime = tempTime;
        }
    }
    if(executeTime != INF)
        cout << "executeTime:" << executeTime << endl;
    return executeTime;
}

