#ifndef DemandController_hpp
#define DemandController_hpp

#include "Web/dto/DTOs.hpp"
#include "Web/svc/NetService.hpp"


#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "Alg/Network.h"
#include "oatpp/core/data/stream/Stream.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"


#include "oatpp/web/mime/multipart/FileProvider.hpp"
#include "oatpp/web/mime/multipart/InMemoryDataProvider.hpp"
#include "oatpp/web/mime/multipart/Reader.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"

namespace multipart = oatpp::web::mime::multipart;

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class DemandController : public oatpp::web::server::api::ApiController {
private:

    NetService* netService=NetService::getInstance();
public:
    /**
     * Constructor with object mapper.
     * @param objectMapper - default object mapper used to serialize/deserialize DTOs.
     */
    DemandController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
    {}
public:

    ENDPOINT("GET", "/api/v1/demands", getDemands) {
        //获取所有需求信息
        auto result=netService->getAllDemands();
        return createDtoResponse(Status::CODE_200, result);
    }

    ENDPOINT("GET", "/api/v1/file/demands", getDemandFiles) {
        //获取../data/demands/目录下所有文件的文件名并返回
        auto result=netService->getDemandFileNames();
        return createDtoResponse(Status::CODE_200,result);
    }

    ENDPOINT("POST", "/api/v1/select/demand/{fileName}", selectDemand,PATH(String,fileName)) {
        //为仿真选择需求文件
        OATPP_LOGI("Select","Select Demand %s",fileName.getValue("").c_str());
        netService->selectDemands(fileName.getValue(""));
        auto result=netService->getAllDemands();
        return createDtoResponse(Status::CODE_200,result);
    }


    ENDPOINT("POST", "api/v1/file/demands/upload", uploadDemandFile, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        //上传需求文件
        auto multipart = std::make_shared<multipart::PartList>(request->getHeaders());
        /* Create multipart reader. */
        multipart::Reader multipartReader(multipart.get());
    
    
        multipartReader.setPartReader("file", multipart::createInMemoryPartReader(-1));
    
        request->transferBody(&multipartReader);
        auto part1 = multipart->getNamedPart("file");

        OATPP_LOGD("Multipart", "parts_count=%d", multipart->count());
    
        
        /* Assert part is not null */
    
        OATPP_ASSERT_HTTP(part1, Status::CODE_400, "file is null");
    
        auto filename  = part1->getFilename();
        OATPP_LOGD("test", "upload file %s", filename.getValue("").c_str());
        
        oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../data/demands/" + filename.getValue("")).c_str());
        fileOutputStream.writeSimple(part1->getPayload()->getInMemoryData());

        auto result=netService->getDemandFileNames();
    
        return createDtoResponse(Status::CODE_200, result);
    }   


};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#endif /* Controller_hpp */