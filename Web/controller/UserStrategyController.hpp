#ifndef USERSTRATEGYCONTROLLER_HPP
#define USERSTRATEGYCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/data/resource/File.hpp"
#include "oatpp/core/data/resource/Resource.hpp"

#include "Web/Utils.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class UserStrategyController : public oatpp::web::server::api::ApiController {
public:
  UserStrategyController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
private:
    NetService* netService=NetService::getInstance();
    OATPP_COMPONENT(std::shared_ptr<StaticFilesManager>, staticFileManager);
public:

  static std::shared_ptr<UserStrategyController> createShared(
    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
  ){
    return std::make_shared<UserStrategyController>(objectMapper);
  }

  ENDPOINT("GET","api/v1/route/allUserStrategy",getAllUserRouteStrategy){
    // template<class T>
    // class ListDto : public oatpp::DTO {

    // DTO_INIT(ListDto, DTO)

    // DTO_FIELD(UInt32, count);
    // DTO_FIELD(Vector<T>, items);

    // };

    // class StrategyDto: public oatpp::DTO{
    //     DTO_INIT(StrategyDto,DTO)

    //     DTO_FIELD(Int32,id);
    //     DTO_FIELD(String,name);
    //     DTO_FIELD(String,filePath);
    //     DTO_FIELD(String,soPath);
    //     DTO_FIELD(String,code);
    // };

    // res is ListDTO<StrategyDTO>
    auto res=netService->getAllUserRouteStrategy();
    return createDtoResponse(Status::CODE_200,res);
  }
  ENDPOINT("GET","api/v1/route/strategyInfo/{id}",getRouteStrategy,PATH(Int32,id)){
    // class StrategyDto: public oatpp::DTO{
    //     DTO_INIT(StrategyDto,DTO)

    //     DTO_FIELD(Int32,id);
    //     DTO_FIELD(String,name);
    //     DTO_FIELD(String,filePath);
    //     DTO_FIELD(String,soPath);
    //     DTO_FIELD(String,code);
    // };
    //res is StrategyDTO
    auto res=netService->getUserRouteStrategyByID(id.getValue(0),true);
    return createDtoResponse(Status::CODE_200,res);
  }
  
  ENDPOINT("POST","api/v1/route/modify-code/{id}",modifyRouteCode,PATH(Int32,id),BODY_DTO(Object<CodeDto>, dto),REQUEST(std::shared_ptr<IncomingRequest>, request)){
        auto strategy=netService->getUserRouteStrategyByID(id.getValue(0),false);
        if(!strategy->filePath||strategy->filePath==""){
            return createResponse(Status::CODE_404,"No Such Strategy Found");
        }

        auto code=dto->code;
        {
            oatpp::data::stream::FileOutputStream fileOutputStream(strategy->filePath->c_str());
            fileOutputStream.writeSimple(code);
        }
        std::string compileCmd = "g++ -std=c++17 -fPIC -shared " + strategy->filePath + " -I../ -L./Alg -lalg -o " + strategy->soPath;
        std::cout<<"Compile: "<<compileCmd.c_str()<<std::endl;
        if (std::system(compileCmd.c_str()) != 0) {
            return createResponse(Status::CODE_500, "编译失败");
        }
        return createResponse(Status::CODE_200, "OK");
  }

  ENDPOINT("POST","api/v1/route/delete/{id}",deleteRouteStrategy,PATH(Int32,id)){
        int success=netService->deleteRouteStrategyByID(id.getValue(0));
        if(success==1){
            return createResponse(Status::CODE_200,"OK");
        }
        return createResponse(Status::CODE_404,"ERROR Not Found");
  }

  ENDPOINT("POST", "api/v1/route/upload-code", uploadRoute, BODY_DTO(Object<CodeDto>, dto),REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        auto fileName=dto->filename;
        auto strategyName=dto->name;
        auto code=dto->code;
        {
            oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../Alg/Route/User/" + fileName.getValue("")).c_str());
            fileOutputStream.writeSimple(code);
        }
        std::string soName="lib"+fileName+".so";
        std::string filePath = "../Alg/Route/User/" + fileName;
        std::string soPath = "../Alg/Route/User/"+soName;
        std::string compileCmd = "g++ -std=c++17 -fPIC -shared " + filePath + " -I../ -L./Alg -lalg -o " + soPath;
        std::cout<<"Compile: "<<compileCmd.c_str()<<std::endl;
        // system("ls -l ../Alg/Route/CustomRouteStrategy.cpp");
        // system("cat ../Alg/Route/CustomRouteStrategy.cpp");
        // if (std::system("g++ -std=c++17 -fPIC -shared ../Alg/Route/CustomRouteStrategy.cpp -I../ -L./Alg -lalg -o ../Alg/Route/libUserRoute.so") != 0) {
        //     return createResponse(Status::CODE_500, "编译失败");
        // }
        if (std::system(compileCmd.c_str()) != 0) {
            return createResponse(Status::CODE_500, "编译失败");
        }
        netService->createRouteStrategy(strategyName,soPath,filePath);
        return createResponse(Status::CODE_200, "OK");
    }   
  ENDPOINT("POST", "api/v1/sched/upload-code", uploadSched, BODY_DTO(Object<CodeDto>, dto),REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        auto fileName=dto->name;
        auto code=dto->code;
        {
            oatpp::data::stream::FileOutputStream fileOutputStream(std::string("../Alg/Sched/" + fileName.getValue("")).c_str());
            fileOutputStream.writeSimple(code);
        }
        
        std::string filePath = "../Alg/Sched/" + fileName;
        std::string soPath = "../Alg/Sched/libUserSched.so";
        std::string compileCmd = "g++ -std=c++17 -fPIC -shared " + filePath + " -I../ -o " + soPath;
        std::cout<<"Compile: "<<compileCmd.c_str()<<std::endl;
        system("ls -l ../Alg/Sched/CustomSchedStrategy.cpp");
        // system("cat ../Alg/Route/CustomRouteStrategy.cpp");
        if (std::system("g++ -std=c++17 -fPIC -shared ../Alg/Sched/CustomSchedStrategy.cpp -I../ -L./Alg -lalg -o ../Alg/Sched/libUserSched.so") != 0) {
            return createResponse(Status::CODE_500, "编译失败");
        }
    
        return createResponse(Status::CODE_200, "OK");
    }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif