#ifndef SimController_hpp
#define SimController_hpp

#include "Web/dto/DTOs.hpp"
#include "Web/svc/NetService.hpp"


#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "Alg/Network.h"



#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class SimController : public oatpp::web::server::api::ApiController {
private:

    NetService* netService=NetService::getInstance();
public:
    /**
     * Constructor with object mapper.
     * @param objectMapper - default object mapper used to serialize/deserialize DTOs.
     */
    SimController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
    {}
public:

    ENDPOINT("POST", "/api/v1/sim/start", simStart,QUERY(Int32, routeAlg, "routingAlgorithm"),QUERY(Int32, scheduleAlg, "schedulingAlgorithm")) {
        //启动
        OATPP_LOGI("Sim","Start with route alg %d, schedule alg %d",routeAlg.getValue(0),scheduleAlg.getValue(0));

        bool success=netService->start(routeAlg.getValue(0),scheduleAlg.getValue(0));
        if(success){
            return createResponse(Status::CODE_200,"OK");
        }else{
            return createResponse(Status::CODE_400,"路由或调度策略的参数非法");
        }

    }

    ENDPOINT("POST", "/api/v1/sim/reset", simReset) {
        //重置
        OATPP_LOGI("Sim","Reset");

        netService->Clear();
        return createResponse(Status::CODE_200,"OK");
    }

    ENDPOINT("POST", "/api/v1/sim/nextStep", nextStep) {
        //下一步
        OATPP_LOGI("Sim","nextStep");

        netService->nextStep();
        auto res=netService->getSimStatus();
        
        return createDtoResponse(Status::CODE_200,res);
    }
    
    ENDPOINT("POST", "/api/v1/sim/next10Step", next10Step) {
        //下十步
        OATPP_LOGI("Sim","nextStep");

        netService->next10Step();
        auto res=netService->getSimStatus();
        
        return createDtoResponse(Status::CODE_200,res);
    }

    ENDPOINT("GET", "/api/v1/sim/res", getSimRes) {
        //下一步
        OATPP_LOGI("Sim","getSimRes");

        auto res=netService->getSimRes();
        
        return createDtoResponse(Status::CODE_200,res);
    }

    ENDPOINT("GET", "/api/v1/sim/status", getSimStatus) {
        //下一步
        OATPP_LOGI("Sim","getSimStatus");

        auto res=netService->getSimStatus();
        
        return createDtoResponse(Status::CODE_200,res);
    }


};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#endif /* Controller_hpp */