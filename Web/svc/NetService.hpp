#ifndef NetService_hpp
#define NetService_hpp
#include <mutex>
#include <memory>
#include<vector>
#include "Alg/Network.h"
#include "Alg/QKDSim.h"
#include "Web/dto/DTOs.hpp"
#include "Web/dto/ListDTO.hpp"
#include "Web/dao/SimDao.hpp"
#include "Config/ConfigReader.h"
class NetService {
private:
    static std::unique_ptr<NetService> instance;
    static std::once_flag initFlag;

    vector<CNetwork*> networks;
    vector<QKDSim*> sims;
    SimDao simDao;
    int groupId;

    // 私有构造函数，确保不能外部创建实例
    NetService():simDao() {
        // std::cout << "Singleton Constructor\n";
        for(int i=0;i<4;i++){
            networks.push_back(new CNetwork());
        }
        for(int i=0;i<4;i++){
            sims.push_back(new QKDSim(networks[i]));
        }
        groupId=rand();
    }

public:
    // 禁止复制和赋值
    NetService(const NetService&) = delete;
    NetService& operator=(const NetService&) = delete;

    // 获取单例实例
    static NetService* getInstance() {
        std::call_once(initFlag, [](){
            instance.reset(new NetService());
        });
        return instance.get();
    }

    // CNetwork 的接口方法
    // CNetwork& getNetwork() {
    //     return network;
    // }

    oatpp::Object<ListDto<oatpp::Object<DemandDto>>> getAllDemands();

    oatpp::Object<PageDto<oatpp::Object<DemandDto>>> getPageDemands(uint32_t offset,uint32_t limit);

    oatpp::Object<PageDto<oatpp::Object<LinkDto>>> getPageLinks(uint32_t offset,uint32_t limit);

    oatpp::Object<ListDto<oatpp::String>> getDemandFileNames();

    oatpp::Object<ListDto<oatpp::String>> getLinkFileNames();

    bool selectDemands(std::string fileName);
    bool selectLinks(std::string fileName);

    oatpp::Object<ListDto<oatpp::Object<SimResDto>>> getSimRes(int route,int sched);

    oatpp::Object<ListDto<oatpp::Object<SimResDto>>> getSimResByStep(int route,int sched,int step);

    oatpp::Object<SimStatusDto> getSimStatus(int route,int sched);
    oatpp::Object<SimResStatusDto> getSimResStatusByStep(int route,int sched,int step);

    void begin(bool on,int routeAlg,int scheduleAlg);

    void nextStep();//unused
    void next10Step();//unused

    void Clear();

    bool start(int routeAlg,int scheduleAlg);

    bool allStart();
};

#endif