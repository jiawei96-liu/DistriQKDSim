#ifndef ControlClient_hpp
#define ControlClient_hpp


#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/core/macro/codegen.hpp"

class ControlClient : public oatpp::web::client::ApiClient {
#include OATPP_CODEGEN_BEGIN(ApiClient)
  
  API_CLIENT_INIT(ControlClient)
  
  //-----------------------------------------------------------------------------------------------
  // Synchronous calls
  
  
  API_CALL("GET", "get", doGet)
  API_CALL("POST", "post", doPost, BODY_STRING(String, body))
  API_CALL("PUT", "put", doPut, BODY_STRING(String, body))
  API_CALL("PATCH", "patch", doPatch, BODY_STRING(String, body))
  API_CALL("DELETE", "delete", doDelete)
  
  
  API_CALL("GET", "anything/{parameter}", doGetAnything, PATH(String, parameter))
  API_CALL("POST", "api/v1/select/link/{fileName}", selectLink, PATH(String, fileName), BODY_STRING(String, body))
  API_CALL("POST", "api/v1/select/demand/{fileName}", selectDemand, PATH(String, fileName), BODY_STRING(String, body))


  API_CALL("POST", "api/v1/sim/start", start,BODY_STRING(String, body),QUERY(Int32, routeAlg, "routingAlgorithm"),QUERY(Int32, scheduleAlg, "schedulingAlgorithm"))
  API_CALL("POST", "api/v1/sim/begin", begin, BODY_STRING(String, body),QUERY(UInt32, on, "on"),QUERY(Int32, routeAlg, "routingAlgorithm"),QUERY(Int32, scheduleAlg, "schedulingAlgorithm"))
  API_CALL("POST","api/v1/sim/reset",reset)


  API_CALL("PUT", "anything/{parameter}", doPutAnything, PATH(String, parameter), BODY_STRING(String, body))
  API_CALL("PATCH", "anything/{parameter}", doPatchAnything, PATH(String, parameter), BODY_STRING(String, body))
  API_CALL("DELETE", "anything/{parameter}", doDeleteAnything, PATH(String, parameter))
  
#include OATPP_CODEGEN_END(ApiClient)
};

#endif /* DemoApiClient_hpp */