#pragma once
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/** 单个域内策略 */
class IntraDomainStrategyDto : public oatpp::DTO {
  DTO_INIT(IntraDomainStrategyDto, DTO)
  DTO_FIELD(Int32, subdomainId);
  DTO_FIELD(Int32, strategy);
};

/** /sim/oneStart 请求体 */
class SimStartRequestDto : public oatpp::DTO {
  DTO_INIT(SimStartRequestDto, DTO)
  DTO_FIELD(Int32, interRoutingStrategy); // 跨域路由（整数: RouteStrategyType）
  DTO_FIELD(List<Object<IntraDomainStrategyDto>>, intraDomainStrategies); // 域内策略列表
  DTO_FIELD(Int32, schedulingAlgorithm);  // 调度算法（整数）
};

/** 通用响应 */
class SimpleRespDto : public oatpp::DTO {
  DTO_INIT(SimpleRespDto, DTO)
  DTO_FIELD(Boolean, success);
  DTO_FIELD(String, message);
};

#include OATPP_CODEGEN_END(DTO)
