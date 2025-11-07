
#ifndef STATICCONTROLLER_HPP
#define STATICCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/data/resource/File.hpp"
#include "oatpp/core/data/resource/Resource.hpp"
#include <filesystem>

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

  
  ENDPOINT("GET", "/public/*", publicFilePage,
         REQUEST(std::shared_ptr<IncomingRequest>, request)) {

    auto tail = request->getPathTail();
    OATPP_ASSERT_HTTP(tail && tail->size() > 0, Status::CODE_400, "Bad Request");

    // URL 解码
    std::string decoded = percentDecode(tail);

    // 基础安全检查：禁止绝对路径与目录回溯
    OATPP_ASSERT_HTTP(!decoded.empty() && decoded[0] != '/', Status::CODE_400, "Absolute path is not allowed");
    OATPP_ASSERT_HTTP(decoded.find("..") == std::string::npos, Status::CODE_400, "Parent path is not allowed");

    // 组合并做 canonical 校验
    std::filesystem::path base = std::filesystem::canonical("../Web/static/public");
    std::filesystem::path target = std::filesystem::weakly_canonical(base / decoded);

    // 防止目录穿越：要求 target 以 base 为前缀
    auto baseStr = base.string();
    auto targetStr = target.string();
    OATPP_ASSERT_HTTP(targetStr.rfind(baseStr, 0) == 0, Status::CODE_400, "Path escapes base directory");

    // 读取文件
    cout<<"file: ../Web/static/public/"+decoded<<endl;
    auto filepath="../Web/static/public/"+decoded;
    auto file = staticFileManager->getFile(filepath);
    OATPP_ASSERT_HTTP(file.get() != nullptr, Status::CODE_404, "File not found");

    auto response = createResponse(Status::CODE_200, file);
    response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_KEEP_ALIVE);

    if (auto mimeType = staticFileManager->guessMimeType(targetStr.c_str())) {
      response->putHeader(Header::CONTENT_TYPE, mimeType);
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

  // [move to UserStrategyController.hpp]

  // ENDPOINT("POST", "api/v1/route/upload-code", uploadRoute, BODY_DTO(Object<CodeDto>, dto),REQUEST(std::shared_ptr<IncomingRequest>, request)) {
  //       auto fileName=dto->filename;
  //       auto strategyName=dto->name;
  //       auto code=dto->code;
  //       {
  //           oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../Alg/Route/" + fileName.getValue("")).c_str());
  //           fileOutputStream.writeSimple(code);
  //       }
  //       std::string soName="lib"+fileName;
  //       std::string filePath = "../Alg/Route/User" + fileName;
  //       std::string soPath = "../Alg/Route/User"+soName;
  //       std::string compileCmd = "g++ -std=c++17 -fPIC -shared " + filePath + " -I../ -L./Alg -lalg -o " + soPath;
  //       std::cout<<"Compile: "<<compileCmd.c_str()<<std::endl;
  //       system("ls -l ../Alg/Route/CustomRouteStrategy.cpp");
  //       // system("cat ../Alg/Route/CustomRouteStrategy.cpp");
  //       // if (std::system("g++ -std=c++17 -fPIC -shared ../Alg/Route/CustomRouteStrategy.cpp -I../ -L./Alg -lalg -o ../Alg/Route/libUserRoute.so") != 0) {
  //       //     return createResponse(Status::CODE_500, "编译失败");
  //       // }
  //       if (std::system(compileCmd.c_str()) != 0) {
  //           return createResponse(Status::CODE_500, "编译失败");
  //       }
  //       return createResponse(Status::CODE_200, "OK");
  //   }   
  // ENDPOINT("POST", "api/v1/sched/upload-code", uploadSched, BODY_DTO(Object<CodeDto>, dto),REQUEST(std::shared_ptr<IncomingRequest>, request)) {
  //       auto fileName=dto->name;
  //       auto code=dto->code;
  //       {
  //           oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../Alg/Sched/" + fileName.getValue("")).c_str());
  //           fileOutputStream.writeSimple(code);
  //       }
        
  //       std::string filePath = "../Alg/Sched/" + fileName;
  //       std::string soPath = "../Alg/Sched/libUserSched.so";
  //       std::string compileCmd = "g++ -std=c++17 -fPIC -shared " + filePath + " -I../ -o " + soPath;
  //       std::cout<<"Compile: "<<compileCmd.c_str()<<std::endl;
  //       system("ls -l ../Alg/Sched/CustomSchedStrategy.cpp");
  //       // system("cat ../Alg/Route/CustomRouteStrategy.cpp");
  //       if (std::system("g++ -std=c++17 -fPIC -shared ../Alg/Sched/CustomSchedStrategy.cpp -I../ -L./Alg -lalg -o ../Alg/Sched/libUserSched.so") != 0) {
  //           return createResponse(Status::CODE_500, "编译失败");
  //       }
    
  //       return createResponse(Status::CODE_200, "OK");
  //   }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif //CRUD_STATICCONTROLLER_HPP
