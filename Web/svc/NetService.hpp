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
class NetService {
private:
    static std::unique_ptr<NetService> instance;
    static std::once_flag initFlag;

    CNetwork network;
    QKDSim sim;
    SimDao simDao;

    // 私有构造函数，确保不能外部创建实例
    NetService():network(),sim(&network),simDao("tcp://127.0.0.1:3306","debian-sys-maint","jEVKZ9vhBRhQ2KHY","QKDSIM_DB") {
        // std::cout << "Singleton Constructor\n";
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
    CNetwork& getNetwork() {
        return network;
    }

    oatpp::Object<ListDto<oatpp::Object<DemandDto>>> getAllDemands();

    oatpp::Object<PageDto<oatpp::Object<DemandDto>>> getPageDemands(uint32_t offset,uint32_t limit);

    oatpp::Object<PageDto<oatpp::Object<LinkDto>>> getPageLinks(uint32_t offset,uint32_t limit);

    oatpp::Object<ListDto<oatpp::String>> getDemandFileNames();

    oatpp::Object<ListDto<oatpp::String>> getLinkFileNames();

    bool selectDemands(std::string fileName);
    bool selectLinks(std::string fileName);

    oatpp::Object<ListDto<oatpp::Object<SimResDto>>> getSimRes();

    oatpp::Object<ListDto<oatpp::Object<SimResDto>>> getSimResByStep(int step);

    oatpp::Object<SimStatusDto> getSimStatus();
    oatpp::Object<SimResStatusDto> getSimResStatusByStep(int step);

    void begin(bool on);

    void nextStep();
    void next10Step();

    void Clear();

    bool start(int routeAlg,int scheduleAlg);
};

#endif