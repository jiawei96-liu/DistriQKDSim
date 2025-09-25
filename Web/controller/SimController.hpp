#ifndef SimController_hpp
#define SimController_hpp

#include "Web/dto/DTOs.hpp"
#include "Web/svc/NetService.hpp"


#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "Alg/Network.h"

#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"



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

    ENDPOINT("POST", "/api/v1/sim/allStart", simAllStart,QUERY(Int32, routeAlg, "routingAlgorithm"),QUERY(Int32, scheduleAlg, "schedulingAlgorithm")) {
        //启动
        OATPP_LOGI("Sim","Start with route alg %d, schedule alg %d",routeAlg.getValue(0),scheduleAlg.getValue(0));

        // bool success=netService->start(routeAlg.getValue(0),scheduleAlg.getValue(0));
        bool success=netService->allStart();
        if(success){
            return createResponse(Status::CODE_200,"OK");
        }else{
            return createResponse(Status::CODE_400,"路由或调度策略的参数非法");
        }

    }

    
    ENDPOINT("POST", "/api/v1/sim/start", simStart,QUERY(Int32, routeAlg, "routingAlgorithm"),QUERY(Int32, scheduleAlg, "schedulingAlgorithm")) {
        //启动
        OATPP_LOGI("Sim","Start with route alg %d, schedule alg %d",routeAlg.getValue(0),scheduleAlg.getValue(0));

        // bool success=netService->start(routeAlg.getValue(0),scheduleAlg.getValue(0));
        int simID=netService->start(routeAlg.getValue(0),scheduleAlg.getValue(0));
        if(simID!=0){
            return createResponse(Status::CODE_200,to_string(simID));
        }else{
            return createResponse(Status::CODE_400,"路由或调度策略的参数非法");
        }

    }

    ENDPOINT("POST", "/api/v1/sim/reset", simReset) {
        //重置
        OATPP_LOGI("Sim","Reset");
        if(ConfigReader::getStr("role") == "master"&&ConfigReader::getStr("worker")!=""){
            // 获取worker的IP和端口
            string workerIp = ConfigReader::getStr("worker");
            unsigned int workerPort = ConfigReader::getInt("worker_port");

            
            auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({workerIp, workerPort});
            auto httpExecutor= oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
            auto client=ControlClient::createShared(httpExecutor,this->getDefaultObjectMapper());

            auto data=client->reset()->readBodyToString();
            // auto data=client->selectLink(fileName.getValue(""),"")->readBodyToString();

            if(data=="OK") {
                OATPP_LOGI("Forward", "Request forwarded to worker successfully.");
            } else {
                OATPP_LOGE("Forward", "Failed to forward request to worker.");
                return createResponse(Status::CODE_500,"worker节点异常");
            }
        }
        netService->Clear();
        return createResponse(Status::CODE_200,"OK");
    }

    ENDPOINT("GET","/api/v1/sim/ResByStep",getSimResByStep,QUERY(UInt32, step, "step"),QUERY(Int32, routeAlg, "routingAlgorithm",0),QUERY(Int32, scheduleAlg, "schedulingAlgorithm",0),QUERY(Int32,simId,"simId",0)){
        OATPP_LOGI("Sim","getSimResByStep");
        auto res=netService->getSimResStatusByStep(routeAlg.getValue(0),scheduleAlg.getValue(0),step.getValue(0),simId.getValue(0));
        return createDtoResponse(Status::CODE_200,res);
    }

    // ENDPOINT("POST", "/api/v1/sim/nextStep", nextStep) {
    //     //下一步
    //     OATPP_LOGI("Sim","nextStep");

    //     netService->nextStep();
    //     auto res=netService->getSimStatus(0,0);
        
    //     return createDtoResponse(Status::CODE_200,res);
    // }
    
    // ENDPOINT("POST", "/api/v1/sim/next10Step", next10Step) {
    //     //下十步
    //     OATPP_LOGI("Sim","nextStep");

    //     netService->next10Step();
    //     auto res=netService->getSimStatus(0,0);
        
    //     return createDtoResponse(Status::CODE_200,res);
    // }

    ENDPOINT("GET", "/api/v1/sim/res", getSimRes) {
        //下一步
        OATPP_LOGI("Sim","getSimRes");

        auto res=netService->getSimRes(0,0);
        
        return createDtoResponse(Status::CODE_200,res);
    }

    // ENDPOINT("GET", "/api/v1/sim/status", getSimStatus) {
    //     //下一步
    //     OATPP_LOGI("Sim","getSimStatus");

    //     auto res=netService->getSimStatus();
        
    //     return createDtoResponse(Status::CODE_200,res);
    // }

    ENDPOINT("GET", "/api/v1/sim/status", getSimStatus,QUERY(Int32, routeAlg, "routingAlgorithm",0),QUERY(Int32, scheduleAlg, "schedulingAlgorithm",0),QUERY(Int32,simId,"simId",0)) {
        //下一步
        OATPP_LOGI("Sim","getSimStatus");

        auto res=netService->getSimStatus(routeAlg.getValue(0),scheduleAlg.getValue(0),simId.getValue(0));
        cout<<"complete get status"<<endl;
        
        return createDtoResponse(Status::CODE_200,res);
    }

    ENDPOINT("POST","/api/v1/sim/begin",setSimStatus,QUERY(UInt32, on, "on"),QUERY(Int32, routeAlg, "routingAlgorithm"),QUERY(Int32, scheduleAlg, "schedulingAlgorithm")){
        OATPP_LOGI("Sim","setSimStatus");
        if(on==0){
            netService->begin(false,routeAlg.getValue(0),scheduleAlg.getValue(0));
        }else if(on==1){
            netService->begin(true,routeAlg.getValue(0),scheduleAlg.getValue(0));
        }else{
            return createResponse(Status::CODE_400,"invalid param");
        }
        return createResponse(Status::CODE_200,"OK");
    }

    ENDPOINT("GET", "/api/v1/sim/history", getAllSims) {
        //下一步
        OATPP_LOGI("Sim","getAllSim");

        auto res=netService->getHistorySims();
        cout<<"complete getAllSim"<<endl;
        
        return createDtoResponse(Status::CODE_200,res);
    }

    ENDPOINT("DELETE", "/api/v1/sim/{simId}", deleteSimById,PATH(Int32,simId)) {
        //下一步
        OATPP_LOGI("Sim","delete sim");

        auto res=netService->deleteSimById(simId.getValue(0));
        cout<<"complete deleteSim"<<endl;
        
        return createResponse(Status::CODE_200,"OK");
    }
    //download sim res
    ENDPOINT("GET", "/api/v1/sim/resDownload", exportSimResFile,
            QUERY(Int32, simId, "simId")) {
        
        OATPP_LOGI("Sim", "Exporting simulation results for simId=%d", simId.getValue(0));
        if(simId==0){
            return createResponse(Status::CODE_400, "Invalid SimId");
        }
        
        std::shared_ptr<std::stringstream> ss = std::make_shared<std::stringstream>();

        // 写入 CSV 表头
        *ss << "Step,CurrentTime,DemandID,NodeID,NextNodeID,NextHopLinkID,AvailableKeys,RemainVolume,Status,IsRouteFailed\n";

        // 调用函数写入内容
        int result = netService->simDao.exportSimResToStream(simId.getValue(0), *ss);
        if (result != 0) {
            return createResponse(Status::CODE_500, "Error exporting simulation results");
        }

        // 创建 StreamingBody 响应
        // auto body = std::make_shared<oatpp::web::protocol::http::outgoing::StreamingBody>(
        //     [ss](const std::shared_ptr<oatpp::data::stream::OutputStream>& stream) {
        //         // stream->write(ss->str().c_str(), ss->str().size());
        //         stream->writeSimple(ss->str());
        //     }
        // );
        auto response=createResponse(Status::CODE_200,ss->str());
        // auto response = OutgoingResponse::createShared(Status::CODE_200, body);
        response->putHeader(Header::CONTENT_TYPE, "text/csv");
        response->putHeader("Content-Disposition", "attachment; filename=\""+to_string(simId.getValue(0))+".csv\"");
        return response;
    }


    //Metric
    ENDPOINT("GET","/api/v1/sim/metrics",getSimMetrics,QUERY(UInt32, step, "step"),QUERY(Int32, routeAlg, "routingAlgorithm",0),QUERY(Int32, scheduleAlg, "schedulingAlgorithm",0),QUERY(Int32,simId,"simId",0)){
        OATPP_LOGI("Sim","getSimResByStep");
        auto res=netService->getSimMetric(routeAlg.getValue(0),scheduleAlg.getValue(0),step.getValue(0),simId.getValue(0));
        return createDtoResponse(Status::CODE_200,res);
    }
};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#endif /* Controller_hpp */