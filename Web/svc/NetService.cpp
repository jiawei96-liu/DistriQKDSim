#include "Web/svc/NetService.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "Web/client/ControlClient.hpp"

#include <dirent.h>  // 用于读取目录
#include <thread>

std::unique_ptr<NetService> NetService::instance = nullptr;
std::once_flag NetService::initFlag;

oatpp::Object<ListDto<oatpp::Object<DemandDto>>> NetService::getAllDemands(){
    auto res = ListDto<oatpp::Object<DemandDto>>::createShared();
    res->items={};
    auto& network=*networks[0];
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

oatpp::Object<PageDto<oatpp::Object<DemandDto>>> NetService::getPageDemands(uint32_t offset=0,uint32_t limit=-1){
    auto res = PageDto<oatpp::Object<DemandDto>>::createShared();
    res->offset=offset;
    res->limit=limit;
    res->items={};
    res->count=(unsigned int)0;
    auto &network=*networks[0];
    for (uint32_t i=offset;i<min(network.m_vAllDemands.size(),((size_t)offset+(size_t)limit));i++){
        auto& dem=network.m_vAllDemands[i];
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
    res->count=network.m_vAllDemands.size();
    // for(int i=0;i<3;i++){
    //     auto obj=DemandDto::createShared();
    //     obj->demandId=i;
    //     res->items->push_back(obj);
    // }
    return res;
}

oatpp::Object<PageDto<oatpp::Object<LinkDto>>> NetService::getPageLinks(uint32_t offset=0,uint32_t limit=-1){
    auto res = PageDto<oatpp::Object<LinkDto>>::createShared();
    res->offset=offset;
    res->limit=limit;
    res->items={};
    res->count=(unsigned int)0;
    auto &network=*networks[0];
    for(uint32_t i=offset;i<min(network.m_vAllLinks.size(),((size_t)offset+(size_t)limit));i++){
        auto &link=network.m_vAllLinks[i];
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
    res->count=network.m_vAllLinks.size();
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
    for(auto sim:sims){
        sim->loadCSV("../data/demands/"+fileName,Demand);
        sim->readCSV(Demand);
    }
    return true;
}

bool NetService::selectLinks(std::string fileName){
    for(auto sim:sims){
        sim->loadCSV("../data/networks/"+fileName,Network);
        sim->readCSV(Network);
    }
    
    OATPP_LOGD("select","read csv end");
    return true;
}

oatpp::Object<ListDto<oatpp::Object<SimResDto>>> NetService::getSimRes(int route,int sched){
    auto res = ListDto<oatpp::Object<SimResDto>>::createShared();
    res->items={};
    res->count=(unsigned int)0;
    auto &network=*networks[route*2+sched];
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

//UnUsed
oatpp::Object<ListDto<oatpp::Object<SimResDto>>> NetService::getSimResByStep(int route,int sched,int step){
    vector<SimResDto> res;
    TIME currentTime;
    auto &network=*networks[route*2+sched];
    simDao.getSimRes(network.simID,step,res,currentTime);

    auto ret = ListDto<oatpp::Object<SimResDto>>::createShared();
    ret->items={};
    for(auto& simRes:res){
        auto _simRes=SimResDto::createShared();
        *_simRes.getPtr()=simRes;
        ret->items->push_back(_simRes);
    }
    ret->count=ret->items->size();
    return ret;
}

oatpp::Object<ListDto<oatpp::Object<SimStatusDto>>> NetService::getHistorySims(){
    vector<SimStatusDto> res;
    simDao.getAllSims(res);

    auto ret = ListDto<oatpp::Object<SimStatusDto>>::createShared();
    ret->items={};
    for(auto& sim:res){
        auto _sim=SimStatusDto::createShared(sim);
        ret->items->push_back(_sim);
    }
    ret->count=ret->items->size();
    return ret;    
}

oatpp::Object<SimStatusDto> NetService::getSimStatus(int route,int sched,int simID){
    auto ret=SimStatusDto::createShared();

    if(simID==0){
        auto &network=*networks[route*2+sched];
        int success=simDao.getSimStatus(network.simID,*ret.getPtr());

        ret->status=network.status;
    }else{
        int success=simDao.getSimStatus(simID,*ret.getPtr());
    }
    

    return ret;
}

oatpp::Object<SimResStatusDto> NetService::getSimResStatusByStep(int route,int sched,int step,int simId){
    
    vector<SimResDto> res;
    TIME currentTime;
    if(simId==0){
        auto &network=*networks[route*2+sched];
        simDao.getSimRes(network.simID,step,res,currentTime);
    }else{
        simDao.getSimRes(simId,step,res,currentTime);
    }
    

    auto ret = SimResStatusDto::createShared();
    ret->items={};
    for(auto& simRes:res){
        auto _simRes=SimResDto::createShared(simRes);
        ret->items->push_back(_simRes);
    }
    ret->count=ret->items->size();
    ret->currentTime=currentTime;
    ret->currentStep=step;
    return ret;
}

oatpp::Object<SimMetricDto> NetService::getSimMetric(int route,int sched,int step,int simId){
    
    SimMetricDto res;
    TIME currentTime;
    if(simId==0){
        auto &network=*networks[route*2+sched];
        // simDao.getSimRes(network.simID,step,res,currentTime);
        simDao.getSimMetric(network.simID,step,res);
    }else{
        simDao.getSimMetric(simId,step,res);
    }
    
    auto ret=SimMetricDto::createShared(res);
    return ret;
}

void NetService::nextStep(){
    // if (!network.AllDemandsDelivered())
    // {
    //     TIME executeTime = network.OneTimeRelay();
    //     network.MoveSimTime(executeTime);
    // }
}

void NetService::next10Step(){
    for(int i=0;i<10;i++){
        nextStep();
    }
}

int NetService::start(int routeAlg,int scheduleAlg){

    auto &network=*networks[routeAlg*2+scheduleAlg];
    // simDao.clear();
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
    // network.InitRelayPath(std::thread::hardware_concurrency());
    network.InitRelayPath();
    network.InitLinkDemand();
    int success=simDao.createSim(network.simID,groupId,"sim-"+to_string(network.simID),to_string(routeAlg),to_string(scheduleAlg));
    if (success!=1)
    {
        cout<<"Failed to create sim in db"<<endl;
        return false;
    }

    std::thread backgroundThread(&CNetwork::RunInBackGround, &network);
    backgroundThread.detach();
    
    return network.simID;
}

bool NetService::allStart(){
    if(ConfigReader::getStr("role")=="master"&&ConfigReader::getStr("worker")!=""){
        //
        for(int scheduleAlg=0;scheduleAlg<2;scheduleAlg++){
            start(0,scheduleAlg);
        }
        for(int scheduleAlg=0;scheduleAlg<2;scheduleAlg++){
            begin(true,0,scheduleAlg);
        }

        string workerIp = ConfigReader::getStr("worker");
        unsigned int workerPort = ConfigReader::getInt("worker_port");
        for(int scheduleAlg=0;scheduleAlg<2;scheduleAlg++){
            auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({workerIp, workerPort});
            auto httpExecutor= oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
            auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
            auto client=ControlClient::createShared(httpExecutor,objectMapper);
            auto resp=client->start("",1,scheduleAlg);
            string simIdStr=resp->readBodyToString();
            if(resp->getStatusCode()!=200||simIdStr=="0"){
                //TODO:
                cerr<<"worker sim start() error"<<endl;
            }else{
                int simId=stoi(simIdStr);
                networks[2+scheduleAlg]->simID=simId;
            }
            
        }
        for(int scheduleAlg=0;scheduleAlg<2;scheduleAlg++){
            auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({workerIp, workerPort});
            auto httpExecutor= oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
            auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
            auto client=ControlClient::createShared(httpExecutor,objectMapper);
            auto data=client->begin("",1,1,scheduleAlg)->readBodyToString();
            begin(true,0,scheduleAlg);
            if(data!="OK"){
                //TODO:
                cerr<<"worker sim begin() error"<<endl;
            }
        }
        
    }
    else{
        //单节点
        for(int routeAlg=0;routeAlg<2;routeAlg++){
            for(int scheduleAlg=0;scheduleAlg<2;scheduleAlg++){
                start(routeAlg,scheduleAlg);
            }
        }
        for(int routeAlg=0;routeAlg<2;routeAlg++){
            for(int scheduleAlg=0;scheduleAlg<2;scheduleAlg++){
                begin(true,routeAlg,scheduleAlg);
            }
        }
    }
    

    return true;
   
}

void NetService::begin(bool on,int routeAlg,int scheduleAlg){
    SimStatusDto status;
    auto& network=*networks[routeAlg*2+scheduleAlg];
    simDao.getSimStatus(network.simID,status);
    if(status.status=="Complete"){
        return;
    }
    {
        if(on){
            std::unique_lock<std::mutex> lock(network.mtx);
            network.status="Running";
            network.cv.notify_all();
            // int success=simDao.setSimStatus(network.simID,"Running");
            // if(success==1){
                // std::cout<<"network.status=Running"<<std::endl;
            // }else{
            //     network.status=originStatus;
            // }
        }else{
            std::unique_lock<std::mutex> lock(network.mtx);
            network.status="Pause";
            // int success=simDao.setSimStatus(network.simID,"Pause");
            // if(success=1){
            //     network.cv.notify_all();
            // }else{
            //     network.status=originStatus;
            // }
        }
    }
}

void NetService::Clear(){
    for(int i=0;i<4;i++){
        {
            std::unique_lock<std::mutex> lock(networks[i]->mtx);
            networks[i]->status="End";
            simDao.setSimStatus(networks[i]->simID,"End");
            networks[i]->Clear();
            delete networks[i];
        }
    }

    networks.clear();
    sims.clear();
    for(int i=0;i<4;i++){
        networks.push_back(new CNetwork());
    }
    for(int i=0;i<4;i++){
        sims.push_back(new QKDSim(networks[i]));
    }
    groupId=rand();


    // network.Clear();
}

int NetService::deleteSimById(int simId){
    simDao.deleteSimulations(simId);
}