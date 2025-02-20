#include "Web/svc/NetService.hpp"
#include <dirent.h>  // 用于读取目录

std::unique_ptr<NetService> NetService::instance = nullptr;
std::once_flag NetService::initFlag;

oatpp::Object<ListDto<oatpp::Object<DemandDto>>> NetService::getAllDemands(){
    auto res = ListDto<oatpp::Object<DemandDto>>::createShared();
    res->items={};
    res->count=network.m_vAllDemands.size();
    for (auto& dem:network.m_vAllDemands){
        auto obj=DemandDto::createShared();
        obj->demandId=dem.GetDemandId();
        obj->sourceId=dem.GetSourceId();
        obj->sinkId=dem.GetSinkId();
        obj->arriveTime=dem.GetArriveTime();
        obj->completeTime=dem.GetCompleteTime();
        obj->demandVolume=dem.GetDemandVolume();
        obj->remainingVolume=dem.GetRemainingVolume();
        obj->deliveredVolume=dem.GetDeliveredVolume();
        obj->allDelivered=dem.GetAllDelivered();
        obj->routeFailed=dem.GetRoutedFailed();
        res->items->push_back(obj);
    }
    // for(int i=0;i<3;i++){
    //     auto obj=DemandDto::createShared();
    //     obj->demandId=i;
    //     res->items->push_back(obj);
    // }
    return res;
}

oatpp::Object<ListDto<oatpp::Object<LinkDto>>> NetService::getAllLinks(){
    auto res = ListDto<oatpp::Object<LinkDto>>::createShared();
    res->items={};
    res->count=network.m_vAllLinks.size();
    for (auto& link:network.m_vAllLinks){
        auto obj=LinkDto::createShared();
        obj->linkId=link.GetLinkId();
        obj->sourceId=link.GetSourceId();
        obj->sinkId=link.GetSinkId();
        obj->qkdRate=link.GetQKDRate();
        obj->delay=link.GetLinkDelay();
        obj->bandwidth=link.GetBandwidth();
        obj->faultTime=link.GetFaultTime();
        obj->weight=link.GetWeight();
        res->items->push_back(obj);
    }
    return res;
}

oatpp::Object<ListDto<oatpp::String>> NetService::getDemandFileNames(){
    std::vector<std::string> fileNames;
    auto res=ListDto<oatpp::String>::createShared();
    res->items={};
    unsigned int count=0;
    res->count=count;
    DIR* dir = opendir("../data/demands/");  // 打开目标目录
    if (dir == nullptr) {
        return res;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {  // 如果是普通文件
            fileNames.push_back(entry->d_name);
            res->items->push_back(entry->d_name);
            count++;
        }
    }

    closedir(dir);  // 关闭目录
    res->count=count;
    return res;
}

oatpp::Object<ListDto<oatpp::String>> NetService::getLinkFileNames(){
    std::vector<std::string> fileNames;
    auto res=ListDto<oatpp::String>::createShared();
    res->items={};
    unsigned int count=0;
    res->count=count;
    DIR* dir = opendir("../data/networks/");  // 打开目标目录
    if (dir == nullptr) {
        return res;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {  // 如果是普通文件
            fileNames.push_back(entry->d_name);
            res->items->push_back(entry->d_name);
            count++;
        }
    }

    closedir(dir);  // 关闭目录
    res->count=count;
    return res;
}

bool NetService::selectDemands(std::string fileName){
    sim.loadCSV("../data/demands/"+fileName,Demand);
    sim.readCSV(Demand);
    return true;
}

bool NetService::selectLinks(std::string fileName){
    OATPP_LOGD("select","load csv begin");
    sim.loadCSV("../data/networks/"+fileName,Network);
    OATPP_LOGD("select","load csv end");
    sim.readCSV(Network);
    OATPP_LOGD("select","read csv end");
    return true;
}

oatpp::Object<ListDto<oatpp::Object<SimResDto>>> NetService::getSimRes(){
    auto res = ListDto<oatpp::Object<SimResDto>>::createShared();
    res->items={};
    res->count=(unsigned int)0;
    for (NODEID nodeId = 0; nodeId < network.GetNodeNum(); nodeId++)
    {
        for (auto demandIter = network.m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != network.m_vAllNodes[nodeId].m_mRelayVolume.end();)
        {
            if (network.m_vAllDemands[demandIter->first].GetRoutedFailed())
            {
                demandIter = network.m_vAllNodes[nodeId].m_mRelayVolume.erase(demandIter);
                continue;
            }
            if (network.m_vAllDemands[demandIter->first].GetArriveTime() > network.CurrentTime()) // 不显示未到达的需求
            {
                demandIter++;
                continue;
            }

            auto obj=SimResDto::createShared();
            

            DEMANDID demandId = demandIter->first;
            VOLUME relayVolume = demandIter->second;
            bool isDelivered = network.m_vAllDemands[demandId].GetAllDelivered();
            // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
            NODEID nextNode = network.m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
            // 找到当前节点和下一个节点之间的链路 minLink
            LINKID minLink = network.m_mNodePairToLink[make_pair(nodeId, nextNode)];
            VOLUME avaiableKeys = network.m_vAllLinks[minLink].GetAvaialbeKeys();
            TIME completeTime = network.m_vAllDemands[demandId].GetCompleteTime();
            bool isRouteFailed = network.m_vAllDemands[demandId].GetRoutedFailed();
            bool isWait = network.m_vAllLinks[minLink].wait_or_not;

            obj->demandId=demandId;
            obj->nodeId=nodeId;
            obj->nextNodeId=nextNode;
            obj->nextHopLinkId=minLink;
            obj->availableKeys=avaiableKeys;
            obj->remainVolume=relayVolume;
            obj->status=isDelivered?("已完成于("+std::to_string(completeTime)+")"):(isWait?"等待中":"正在传输");
            obj->isRouteFailed=isRouteFailed?"是":"否";

            res->items->push_back(obj);

            demandIter++;
        }
    }
    // 显示已传输完毕的数据
    for (auto demandIter = network.m_vAllDemands.begin(); demandIter != network.m_vAllDemands.end(); demandIter++)
    {
        if (demandIter->GetAllDelivered())
        {
            NODEID nodeId = demandIter->GetSinkId();
            DEMANDID demandId = demandIter->GetDemandId();
            VOLUME relayVolume = 0;
//            bool isDelivered = demandIter->GetAllDelivered();
//            NODEID nextNode = demandIter->GetSinkId();
//            LINKID minLink = network.m_mNodePairToLink[make_pair(nodeId, nextNode)];
//            VOLUME avaiableKeys = network.m_vAllLinks[minLink].GetAvaialbeKeys();
            bool isRouteFailed = network.m_vAllDemands[demandId].GetRoutedFailed();
            TIME completeTime = network.m_vAllDemands[demandId].GetCompleteTime();
            
            auto obj=SimResDto::createShared();
            obj->demandId=demandId;
            obj->nodeId=nodeId;
            obj->remainVolume=relayVolume;
            obj->status=("已完成于("+std::to_string(completeTime)+")");
            obj->isRouteFailed=isRouteFailed?"是":"否";

            res->items->push_back(obj);
        }
    }
    res->count=res->items->size();
    return res;
}

oatpp::Object<SimStatusDto> NetService::getSimStatus(){
    auto simRes=getSimRes();
    auto res=SimStatusDto::createShared();
    res->items=simRes->items;
    res->count=simRes->count;
    res->currentTime=network.CurrentTime();
    res->currentStep=network.CurrentStep();
    return res;
}

void NetService::nextStep(){
    if (!network.AllDemandsDelivered())
    {
        TIME executeTime = network.OneTimeRelay();
        network.MoveSimTime(executeTime);

    }
}

void NetService::next10Step(){
    for(int i=0;i<10;i++){
        nextStep();
    }
}

bool NetService::start(int routeAlg,int scheduleAlg){
    if(routeAlg==0){
        network.setShortestPath();
    }else if (routeAlg==1)
    {
        network.setKeyRateShortestPath();
    }else{
        return false;
    }

    if(scheduleAlg==0){
        network.setMinimumRemainingTimeFirst();
    }else if (scheduleAlg==1){
        network.setAverageKeyScheduling();
    }else{
        return false;
    }
    
    network.InitRelayPath();
    return true;
}

void NetService::Clear(){
    network.Clear();
}