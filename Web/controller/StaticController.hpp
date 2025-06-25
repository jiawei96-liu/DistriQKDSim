
#ifndef STATICCONTROLLER_HPP
#define STATICCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/data/resource/File.hpp"
#include "oatpp/core/data/resource/Resource.hpp"

#include "Web/Utils.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class StaticController : public oatpp::web::server::api::ApiController {
public:
  StaticController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
private:
    OATPP_COMPONENT(std::shared_ptr<StaticFilesManager>, staticFileManager);
public:

  static std::shared_ptr<StaticController> createShared(
    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
  ){
    return std::make_shared<StaticController>(objectMapper);
  }

  ENDPOINT("GET", "/", root) {
    oatpp::String filePath = "../Web/static/index.html";
    auto file = staticFileManager->getFile(filePath);

    OATPP_ASSERT_HTTP(file.get() != nullptr, Status::CODE_404, "File not found");

    std::shared_ptr<OutgoingResponse> response;

    response = createResponse(Status::CODE_200, file);

    // response->putHeader("Accept-Ranges", "bytes");
    response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_KEEP_ALIVE);
    auto mimeType = staticFileManager->guessMimeType(filePath);
    if(mimeType) {
        response->putHeader(Header::CONTENT_TYPE, mimeType);
    } else {
        OATPP_LOGD("Server", "Unknown Mime-Type. Header not set");
    }

    return response;
  }

  ENDPOINT("GET", "/{fileName}", filePage,PATH(String,fileName)) {
    OATPP_ASSERT_HTTP(fileName.getValue("")!="",Status::CODE_400,"Bad Request");
    oatpp::String filePath = "../Web/static/"+fileName.getValue("");
    auto file = staticFileManager->getFile(filePath);

    OATPP_ASSERT_HTTP(file.get() != nullptr, Status::CODE_404, "File not found");

    std::shared_ptr<OutgoingResponse> response;

    response = createResponse(Status::CODE_200, file);

    // response->putHeader("Accept-Ranges", "bytes");
    response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_KEEP_ALIVE);
    auto mimeType = staticFileManager->guessMimeType(filePath);
    if(mimeType) {
        response->putHeader(Header::CONTENT_TYPE, mimeType);
    } else {
        OATPP_LOGD("Server", "Unknown Mime-Type. Header not set");
    }

    return response;
  }
  ENDPOINT("POST", "api/v1/route/upload-code", uploadRoute, BODY_DTO(Object<CodeDto>, dto),REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        auto fileName=dto->name;
        auto code=dto->code;
        
        oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../Alg/Route/" + fileName.getValue("")).c_str());
        fileOutputStream.writeSimple(code);

    
        return createResponse(Status::CODE_200, "OK");
    }   

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif //CRUD_STATICCONTROLLER_HPP