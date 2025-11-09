#include "AvgStrategy.h"
#include "Alg/Network.h"

using namespace sched;

/**
 * 平均密钥调度算法
 * 为指定节点调度中继需求，采用平均分配密钥资源的策略
 * 
 * @param nodeId 当前要调度的节点ID
 * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
 * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
 */
TIME AvgStrategy::Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) {
    // 创建一个随机数引擎 - 用户可修改：可以更换随机数生成算法
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
    std::uniform_int_distribution<> dis(0, 1); // 定义一个分布，生成 0 或 1

    // 生成随机数 - 用户可修改：可以调整随机决策逻辑
    int tempWait = dis(gen);

    // cout<<"进入平均密钥调度算法"<<endl;
    TIME executeTime = INF;                // 表示当前的最小执行时间，初始化为无穷大
    
    // 获取网络相关数据引用 - 用户可修改：可以根据实际网络结构调整
    auto& m_vAllLinks = net->m_vAllLinks;          // 所有链路集合
    auto& m_dSimTime = net->m_dSimTime;            // 当前仿真时间
    auto& m_vAllDemands = net->m_vAllDemands;      // 所有需求集合
    auto& m_vAllNodes = net->m_vAllNodes;          // 所有节点集合
    
    // 遍历所有链路，寻找与当前节点相关的链路
    for(auto &link : m_vAllLinks)
    {
        // 只遍历链路首或尾是该节点的链路 - 用户可修改：可以调整节点连接判断条件
        if (link.GetSourceId() != nodeId && link.GetSinkId() != nodeId)
        {
            continue;  // 跳过与该节点无关的链路
        }
        // cout<<"该节点存在链路"<<endl;
        
        // 跳过故障link和link上没需求的link - 用户可修改：可以调整链路可用性判断条件
        // if (link.GetFaultTime() >= 0 && link.GetFaultTime() <= m_dSimTime || link.m_lCarriedDemands.empty())
        if (link.GetFaultTime() > 0 && link.GetFaultTime() <= m_dSimTime || link.m_lCarriedDemands.empty())
        {
            link.wait_or_not = true;  // 标记链路为等待状态
            continue;
        }
        
        // 获取链路相关信息 - 用户可修改：可以调整密钥和带宽获取方式
        VOLUME availableKeyVolume = link.GetAvaialbeKeys();  // 可用密钥量
        //-------------------------修改部分-------------------------
        //RATE bandwidth = link.GetBandwidth();  // 原总带宽
        RATE classicalbandwidth = link.GetClassicalBandwidth();      // 经典带宽
        //--------------------------------------------------
        
        NODEID sourceid = link.GetSourceId();  // 链路源节点
        NODEID sinkid = link.GetSinkId();      // 链路目的节点
        TIME tempTime = INF;                   // 当前链路的估计执行时间
        int num_of_demand = 0;                 // 当前链路上可执行的需求数量
        
        // 遍历link上的demand，计算链路上的最小执行时间
        for (auto &demandid : link.m_lCarriedDemands)
        {
            // cout<<demandid<<endl;
            // 检查需求是否已经到达 - 用户可修改：可以调整需求到达时间判断条件
            if (m_vAllDemands[demandid].GetArriveTime() > m_dSimTime + SMALLNUM)
            {
                // this demand has not arrived yet
                continue;  // 需求尚未到达，跳过
            }
            // cout<<"该链路存在需求"<<endl;
            num_of_demand++;  // 统计可执行的需求数量
            cout<< "平均密钥调度:"<< link.GetLinkId()<< "have demand" << demandid<<endl;
            
            NODEID nodeid;
            VOLUME relayVolume;
            auto &nextNode = m_vAllDemands[demandid].m_Path.m_mNextNode;
            
            // 确定中继节点和中继数据量 - 用户可修改：可以调整中继节点选择策略
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

            // 对一个demand，判断链路最小执行时间tempTime - 用户可修改：可以调整执行时间计算方式
            if (relayVolume / classicalbandwidth < tempTime)
            {
                tempTime = relayVolume / classicalbandwidth;
            }
            // cout << "relayVolume" << relayVolume<<endl;
            // cout << "classicalbandwidth" << classicalbandwidth<<endl;
            // cout << "tempTime" << tempTime<<endl;
        }
        
        // 没有可以传的需求，继续检查下一条链路
        if (num_of_demand == 0)
        {
            continue;
        }
        
        // 计算需要的总密钥量 - 用户可修改：可以调整密钥需求计算方式
        // VOLUME needVolume = tempTime * classicalbandwidth * link.m_lCarriedDemands.size();
        VOLUME needVolume = tempTime * classicalbandwidth * num_of_demand;
        // cout << "needVolume" << needVolume<<endl;
        // cout << "availableKeyVolume" << availableKeyVolume<<endl;
        
        // 情况1：可用密钥量足够支持所有需求 - 用户可修改：可以调整带宽分配策略
        if (needVolume <= availableKeyVolume)
        {
            link.wait_or_not = false;  // 链路可以立即执行
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
                    relayDemands[demandid] = tempTime * classicalbandwidth;
                }
            }
            //------------------添加部分------------------
            // 用户可修改：可以调整带宽分配比例
            link.SetClassicalBandwidth(0.7); // 设置经典带宽为总带宽的70%
            link.SetNegotiationBandwidth();  // 设置密钥协商带宽为剩余带宽
            //----------------------------------------
        }
        // 情况2：密钥不足但仍有少量可用密钥 - 用户可修改：可以调整阈值和分配策略
        else if (availableKeyVolume >= 50)  // 50是可调整的阈值
        {
            link.wait_or_not = false;  // 链路可以立即执行
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
                // 对每一个可以传输的demand，给相应的nodeid平均分配可用密钥
                if (nodeid == nodeId)
                {
                    // relayDemands[demandid] = availableKeyVolume / link.m_lCarriedDemands.size();
                    relayDemands[demandid] = availableKeyVolume / num_of_demand;
                }
                //------------------添加部分------------------
                // 用户可修改：可以调整带宽分配比例
                link.SetClassicalBandwidth(0.5); // 设置经典带宽为总带宽的50%
                link.SetNegotiationBandwidth();  // 设置密钥协商带宽为剩余带宽
                //----------------------------------------
            }
        }
        // 情况3：密钥严重不足 - 用户可修改：可以调整等待策略和带宽分配
        else
        {
            link.wait_or_not = true;  // 链路需要等待
            //密钥不足，设置较高的协商带宽占用
            //------------------添加部分------------------
            // 用户可修改：可以调整带宽分配比例
            link.SetClassicalBandwidth(0.3); // 设置经典带宽为总带宽的30%
            link.SetNegotiationBandwidth();  // 设置密钥协商带宽为剩余带宽
            //----------------------------------------
            // 计算需要等待的密钥生成时间
            tempTime = (needVolume - availableKeyVolume) / link.GetQKDRate();
        }
        
        // 用户可修改：可以调整最小执行时间限制
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
        
        // 更新全局最小执行时间
        if (tempTime < executeTime)
        {
            executeTime = tempTime;
        }
    }
    
    // 输出调试信息
    if(executeTime != INF)
        cout << "executeTime:" << executeTime << endl;
        
    return executeTime;
}