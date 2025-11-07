#ifndef TOPOLOGYCONFIGCONTROLLER_HPP
#define TOPOLOGYCONFIGCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "Web/dto/DTOs.hpp"
#include "Web/svc/TopologyConfigService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class TopologyConfigController : public oatpp::web::server::api::ApiController {
public:
  TopologyConfigController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}

public:

  ENDPOINT("PUT", "/api/v1/topology/config", updateTopologyConfig, BODY_DTO(Object<TopologyConfigDto>, dto)) {
    OATPP_ASSERT_HTTP(dto, Status::CODE_400, "Topology config payload is required");
    auto config = TopologyConfigService::instance().updateConfig(dto);
    return createDtoResponse(Status::CODE_200, config);
  }

  ENDPOINT("POST", "/api/v1/topology/generate", topoGenerate,
           BODY_DTO(Object<TopologyGenerateRequestDto>, dto)) {
    auto& svc = TopologyConfigService::instance();

    // -------- 输入校验 --------
    if(!dto->filename){
      auto r = TopologyGenerateResponseDto::createShared();
      r->ok = false; r->exitCode = -1;
      r->message = "filename 不能为空";
      return createDtoResponse(Status::CODE_400, r);
    }
    if (!dto || !dto->domains || dto->domains->size() == 0) {
      auto r = TopologyGenerateResponseDto::createShared();
      r->ok = false; r->exitCode = -1;
      r->message = "domains 不能为空";
      return createDtoResponse(Status::CODE_400, r);
    }
    if (dto->domains->size() > 4) {
      auto r = TopologyGenerateResponseDto::createShared();
      r->ok = false; r->exitCode = -1;
      r->message = "最多支持 4 个子域";
      return createDtoResponse(Status::CODE_400, r);
    }
    const v_int32 D = (v_int32) dto->domains->size();
    if (!dto->inter_edges || dto->inter_edges->size() != D) {
      auto r = TopologyGenerateResponseDto::createShared();
      r->ok = false; r->exitCode = -1;
      r->message = "inter_edges 维度错误，应为 D×D 矩阵";
      return createDtoResponse(Status::CODE_400, r);
    }
    for (v_int32 i=0;i<D;i++) {
      auto row = dto->inter_edges[i];
      if (!row || row->size() != D) {
        auto r = TopologyGenerateResponseDto::createShared();
        r->ok = false; r->exitCode = -1;
        r->message = "inter_edges 的每一行都应为长度 D 的数组";
        return createDtoResponse(Status::CODE_400, r);
      }
    }
    for (v_int32 i=0;i<D;i++) {
      auto dom = dto->domains[i];
      if (!dom || dom->nodes < 0 || dom->edges < 0) {
        auto r = TopologyGenerateResponseDto::createShared();
        r->ok = false; r->exitCode = -1;
        r->message = "每个域的 nodes/edges 需为非负整数";
        return createDtoResponse(Status::CODE_400, r);
      }
      // 边数上限检查
      long long maxEdges = 1LL * dom->nodes * (dom->nodes - 1) / 2;
      if (dom->edges > maxEdges) {
        auto r = TopologyGenerateResponseDto::createShared();
        r->ok = false; r->exitCode = -1;
        r->message = oatpp::String("域内边数超过上限: 域 ") + oatpp::String(std::to_string(i+1).c_str()) +
                     " 最大为 " + oatpp::String(std::to_string(maxEdges).c_str());
        return createDtoResponse(Status::CODE_400, r);
      }

      long long minEdges=1LL * (dom->nodes-1);
      if (dom->edges < minEdges) {
        auto r = TopologyGenerateResponseDto::createShared();
        r->ok = false; r->exitCode = -1;
        r->message = oatpp::String("域内边数过小，无法生成连通图: 域 ") + oatpp::String(std::to_string(i+1).c_str()) +
                     " 最小为 " + oatpp::String(std::to_string(minEdges).c_str());
        return createDtoResponse(Status::CODE_400, r);
      }
    }

    // -------- 调用 Python --------
    // 可放到配置文件/环境变量
    const std::string PYTHON_BIN  = "python3";
    const std::string SCRIPT_PATH = "../Input/domain_gen.py";

    std::string stdoutText, stderrText;
    int exitCode = svc.runPythonDomainGen(PYTHON_BIN, SCRIPT_PATH, dto, stdoutText, stderrText);

    auto resp = TopologyGenerateResponseDto::createShared();
    resp->exitCode  = exitCode;
    resp->stdoutText = stdoutText.c_str();
    resp->stderrText = stderrText.c_str();
    resp->savedAs   = svc.getSavedPath(dto).c_str();

    if (exitCode == 0) {
      resp->ok = true;
      resp->message = "拓扑生成成功";
      return createDtoResponse(Status::CODE_200, resp);
    } else {
      resp->ok = false;
      // 让前端能看到 Python 报错
      resp->message = "拓扑生成失败，请查看 stderrText";
      return createDtoResponse(Status::CODE_500, resp);
    }
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif // TOPOLOGYCONFIGCONTROLLER_HPP
