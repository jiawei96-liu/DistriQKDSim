#ifndef LinkController_hpp
#define LinkController_hpp

#include "Web/dto/DTOs.hpp"
#include "Web/svc/NetService.hpp"
#include "Web/client/ControlClient.hpp"


#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "Alg/Network.h"
#include "oatpp/core/data/stream/Stream.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"


#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include "oatpp/web/mime/multipart/FileProvider.hpp"
#include "oatpp/web/mime/multipart/InMemoryDataProvider.hpp"
#include "oatpp/web/mime/multipart/Reader.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"

namespace multipart = oatpp::web::mime::multipart;

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class LinkController : public oatpp::web::server::api::ApiController {
private:

    NetService* netService=NetService::getInstance();
public:
    /**
     * Constructor with object mapper.
     * @param objectMapper - default object mapper used to serialize/deserialize DTOs.
     */
    LinkController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
    {}
public:

    ENDPOINT("GET", "/api/v1/links", getLinks,QUERY(UInt32, offset, "offset"),QUERY(UInt32, limit, "limit")) {
        //获取所有链路拓扑信息
        netService=NetService::getInstance();
        auto dto = DemandDto::createShared();
        auto result=netService->getPageLinks(offset.getValue(0),limit.getValue(1000));

        return createDtoResponse(Status::CODE_200, result);
    }

    ENDPOINT("GET", "/api/v1/file/links", getLinkFiles) {
        //获取../data/links/目录下所有文件的文件名并返回
        auto result=netService->getLinkFileNames();
        return createDtoResponse(Status::CODE_200,result);
    }

    ENDPOINT("POST", "/api/v1/select/link/{fileName}", selectNetwork,PATH(String,fileName)) {
        //为仿真选择一个链路拓扑文件
        OATPP_LOGI("Select","Select Network %s",fileName.getValue("").c_str());

        if(ConfigReader::getStr("role") == "master"&&ConfigReader::getStr("worker")!=""){
            // 获取worker的IP和端口
            string workerIp = ConfigReader::getStr("worker");
            unsigned int workerPort = ConfigReader::getInt("worker_port");

            
            auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({workerIp, workerPort});
            auto httpExecutor= oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
            auto client=ControlClient::createShared(httpExecutor,this->getDefaultObjectMapper());

            auto data=client->selectLink(fileName.getValue(""),"")->readBodyToString();

            if(data=="OK") {
                OATPP_LOGI("Forward", "Request forwarded to worker successfully.");
            } else {
                OATPP_LOGE("Forward", "Failed to forward request to worker.");
                return createResponse(Status::CODE_500,"worker节点异常");
            }
        }

        netService->selectLinks(fileName.getValue(""));
        // auto result=netService->getAllLinks();
        // return createDtoResponse(Status::CODE_200,result);
        return createResponse(Status::CODE_200,"OK");
    }

    ENDPOINT("POST", "api/v1/file/links/upload", uploadLinks, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        // 上传拓扑文件
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
        
        oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../data/networks/" + filename.getValue("")).c_str());
        fileOutputStream.writeSimple(part1->getPayload()->getInMemoryData());

        auto result=netService->getLinkFileNames();
    
        return createDtoResponse(Status::CODE_200, result);
    }   


};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#endif /* Controller_hpp */