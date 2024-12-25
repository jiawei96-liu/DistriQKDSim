#include "Network.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include<chrono>
#include<unistd.h>
using std::cout;
using std::endl;

void next_step(CNetwork* net);
void next_nstep(CNetwork* net,int n);
void run_all(CNetwork* net);

void readNetTb(CNetwork *net, const std::string &csvFileName) {
    std::ifstream file(csvFileName);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open CSV file " << csvFileName << std::endl;
        return;
    }

    std::string line;
    int rowIndex = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> tokens;
        std::string token;

        // 按逗号分割
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (rowIndex == 0) {
            // 第一行表示节点数量
            if (tokens.empty() || tokens[0].empty()) {
                std::cerr << "Error: Missing node number in the first row" << std::endl;
                return;
            }
            UINT nodeNum = static_cast<UINT>(std::stoul(tokens[0]));
            net->SetNodeNum(nodeNum);
            net->InitNodes(nodeNum);
            cout<<"node number: "<<nodeNum<<endl;
        } else {
            // 从第二行开始是链路信息
            if (tokens.size() < 7) {
                std::cerr << "Error: Incomplete data in row " << rowIndex << std::endl;
                continue;
            }

            LINKID linkId = static_cast<LINKID>(std::stoul(tokens[0]));
            NODEID sourceId = static_cast<NODEID>(std::stoul(tokens[1]));
            NODEID sinkId = static_cast<NODEID>(std::stoul(tokens[2]));
            RATE keyRate = static_cast<RATE>(std::stod(tokens[3]));
            TIME proDelay = static_cast<TIME>(std::stod(tokens[4]));
            RATE bandWidth = static_cast<RATE>(std::stod(tokens[5]));
            WEIGHT weight = static_cast<WEIGHT>(std::stod(tokens[6]));
            TIME faultTime = (tokens.size() > 7 && !tokens[7].empty()) ? static_cast<TIME>(std::stod(tokens[7])) : -1;

            // 处理链路信息
            CLink newLink;
            newLink.SetLinkId(linkId);
            newLink.SetSourceId(sourceId);
            newLink.SetSinkId(sinkId);
            newLink.SetQKDRate(keyRate);
            newLink.SetLinkDelay(proDelay);
            newLink.SetBandwidth(bandWidth);
            newLink.SetWeight(weight);
            newLink.SetFaultTime(faultTime);

            net->m_vAllLinks.push_back(newLink);
            net->m_mNodePairToLink[{sourceId, sinkId}] = linkId;
            net->m_mNodePairToLink[{sinkId, sourceId}] = linkId;

            net->m_vAllNodes[sourceId].m_lAdjNodes.push_back(sinkId);
            net->m_vAllNodes[sinkId].m_lAdjNodes.push_back(sourceId);

            int id_faultTime = 1000000 + linkId;
            if (faultTime > 0) {
                net->m_mDemandArriveTime.insert({faultTime, id_faultTime});
            }
        }

        rowIndex++;
        cout<<"row number:"<<rowIndex<<endl;
    }

    net->SetLinkNum(rowIndex - 1); // 第一行是节点数量，需要减 1
    file.close();
    std::cout << "Network data processed successfully from " << csvFileName << std::endl;
}


void readDemTb(CNetwork *net, const std::string &csvFileName) {
    std::ifstream file(csvFileName);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open CSV file " << csvFileName << std::endl;
        return;
    }

    std::string line;
    int rowIndex = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> tokens;
        std::string token;

        // 按逗号分割，去除空格
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
            cout<<token<<" "<<token.size()<<endl;
            // for (char c:token){
                // cout<<int(c)<<" ";
            // }
            cout<<endl;
        }

        // 确保列数足够
        if (tokens.size() < 5) {
            std::cerr << "Error: Incomplete data in row " << rowIndex << std::endl;
            continue;
        }


        // 转换为相应的数据类型
        DEMANDID demandId = static_cast<DEMANDID>(std::stoul(tokens[0]));
        NODEID sourceId = static_cast<NODEID>(std::stoul(tokens[1]));
        NODEID sinkId = static_cast<NODEID>(std::stoul(tokens[2]));
        VOLUME demandVolume = static_cast<VOLUME>(std::stod(tokens[3]));
        
        TIME arriveTime = static_cast<TIME>(std::stod(tokens[4]));
        // 处理需求信息
        CDemand newDemand;
        newDemand.SetDemandId(demandId);
        newDemand.SetSourceId(sourceId);
        newDemand.SetSinkId(sinkId);
        newDemand.SetDemandVolume(demandVolume);
        newDemand.SetRemainingVolume(demandVolume);
        newDemand.SetArriveTime(arriveTime);
        newDemand.SetCompleteTime(INF); // 假设 INF 是一个定义好的常量，表示无限大

        net->m_vAllDemands.push_back(newDemand);
        net->m_vAllNodes[sourceId].m_mRelayVolume[demandId] = demandVolume; // 初始化 m_mRelayVolume
        net->m_mDemandArriveTime.insert({arriveTime, demandId}); // 增加 m_mDemandArriveTime

        rowIndex++;
    }

    net->SetDemandNum(rowIndex); // CSV 文件中行数即为需求数量
    file.close();
    std::cout << "Demand data processed successfully from " << csvFileName << std::endl;
}



int main(){
    CNetwork* net= new CNetwork();
    // net->setKeyRateShortestPath();
    net->setShortestPath();
    // net->setLBPath();
    // net->setKeyRateShortestPath();
    net->setAverageKeyScheduling();
    readNetTb(net,"../Input/100规模/network.csv");
    readDemTb(net,"../Input/100规模/demand.csv");
    // for(auto& n:net->m_vAllNodes){
    //     cout<<"node "<<n.GetNodeId()<<"-----";
    //     cout<<n.
    // }
    for (auto& l:net->m_vAllLinks){
        cout<<"link "<<l.GetLinkId()<<" ----"<<endl;
        cout<<l.GetLinkId()<<","<<l.GetSourceId()<<","<<l.GetSinkId()<<","<<l.GetQKDRate()<<","<<l.GetLinkDelay()<<","<<l.GetBandwidth()<<","<<l.GetWeight()<<","<<l.GetFaultTime()<<endl;
        cout<<"----"<<endl;
    }

    for (auto& d:net->m_vAllDemands){
        cout<<"demand "<<d.GetDemandId()<<" ----"<<endl;
        cout<<d.GetDemandId()<<","<<d.GetSourceId()<<","<<d.GetSinkId()<<","<<d.GetDemandVolume()<<","<<d.GetRemainingVolume()<<","<<d.GetArriveTime()<<endl;
        cout<<"----"<<endl;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    net->InitRelayPath();
    net->InitLinkDemand();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "InitRelayPath execution time: " << duration << " ms" << std::endl;

    // net->MainProcess();
    // net->
    // next_step(net);
    start = std::chrono::high_resolution_clock::now();
    
    next_nstep(net,1);
    // run_all(net);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "1 step execution time: " << duration << " ms" << std::endl;
    // cout<<net->m_vAllDemands.size()<<endl;
    // cout<<net->m_vAllLinks.size()<<endl;
}
// InitRelayPath->InitLinkDemand

// executeTime=OneTimeRelay()->MoveSimTime(executeTime)
void next_step(CNetwork* net)
{
    if (!net->AllDemandsDelivered())
    {
        TIME executeTime = net->OneTimeRelay();
        net->MoveSimTime(executeTime);

        //        showOutput();
    }
    else
    {
        cout<<"All demand have been delivered"<<endl;
    }
}

void next_nstep(CNetwork* net,int n){
    for(int i=0;i<n;i++){
        next_step(net);
    }
}

void run_all(CNetwork* net){
    int i=0;
    while (!net->AllDemandsDelivered())
    {
        // cout<<"Step "<<i<<"-----"<<endl;
        // auto start = std::chrono::high_resolution_clock::now();
        TIME executeTime = net->OneTimeRelay();
        net->MoveSimTime(executeTime);
        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        //        showOutput();
        // std::cout << "one step time: " << duration << " ms" << std::endl;
        // sleep(1);
        //i++
        
    }
    cout<<"All demand have been delivered"<<endl;
}