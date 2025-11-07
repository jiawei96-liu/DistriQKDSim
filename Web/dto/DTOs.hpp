#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class DemandDto : public oatpp::DTO {
  
    DTO_INIT(DemandDto, DTO)

    DTO_FIELD(UInt32, demandId);
    DTO_FIELD(UInt32, sourceId);
    DTO_FIELD(UInt32, sinkId);
    DTO_FIELD(Float64, arriveTime);
    DTO_FIELD(Float64, completeTime);
    DTO_FIELD(Float64, demandVolume);
    DTO_FIELD(Float64, remainingVolume);
    DTO_FIELD(Float64, deliveredVolume);
    DTO_FIELD(Boolean, allDelivered);
    DTO_FIELD(Boolean,routeFailed);

  
  
};

class LinkDto : public oatpp::DTO{
    DTO_INIT(LinkDto,DTO)

    DTO_FIELD(UInt32, linkId);
    DTO_FIELD(UInt32, sourceId);
    DTO_FIELD(UInt32, sinkId);
    DTO_FIELD(Float64, qkdRate);
    DTO_FIELD(Float64, delay);
    DTO_FIELD(Float64, bandwidth);
    DTO_FIELD(Float64, faultTime);
    DTO_FIELD(Float64, weight);
};

class SimResDto: public oatpp::DTO{
    DTO_INIT(SimResDto,DTO)

    DTO_FIELD(UInt32, demandId);
    DTO_FIELD(UInt32, nodeId);
    DTO_FIELD(UInt32, nextNodeId);
    DTO_FIELD(UInt32, nextHopLinkId);
    DTO_FIELD(Float64, availableKeys);
    DTO_FIELD(Float64, remainVolume);
    DTO_FIELD(String,status);
    DTO_FIELD(String,isRouteFailed);

    SimResDto()=default;
    SimResDto(const SimResDto& that){
        demandId=that.demandId;
        nodeId=that.nodeId;
        nextNodeId=that.nextNodeId;
        nextHopLinkId=that.nextHopLinkId;
        availableKeys=that.availableKeys;
        remainVolume=that.remainVolume;
        status=that.status;
        isRouteFailed=that.isRouteFailed;
    }
    
};

class SimStatusDto: public oatpp::DTO{
    DTO_INIT(SimStatusDto,DTO)

    DTO_FIELD(UInt32,id);    
    DTO_FIELD(String,name);
    DTO_FIELD(String,createTime);
    DTO_FIELD(String,status);
    DTO_FIELD(String,routeAlg);
    DTO_FIELD(String,scheduleAlg);
    DTO_FIELD(UInt32,currentStep);
    DTO_FIELD(Float64,currentTime);

    SimStatusDto()=default;
    SimStatusDto(const SimStatusDto& that){
        id=that.id;
        name=that.name;
        createTime=that.createTime;
        status=that.status;
        routeAlg=that.routeAlg;
        scheduleAlg=that.scheduleAlg;
        currentStep=that.currentStep;
        currentTime=that.currentTime;
    }
};

class CodeDto : public oatpp::DTO {
    DTO_INIT(CodeDto, DTO)

    DTO_FIELD(String, name);
    DTO_FIELD(String,filename);
    DTO_FIELD(String, code);
};

class StrategyDto: public oatpp::DTO{
    DTO_INIT(StrategyDto,DTO)

    DTO_FIELD(Int32,id);
    DTO_FIELD(String,name);
    DTO_FIELD(String,filePath);
    DTO_FIELD(String,soPath);
    DTO_FIELD(String,code);
};

class SimMetricDto: public oatpp::DTO{
    DTO_INIT(SimMetricDto,DTO)

    DTO_FIELD(Int32,stepId);    
    DTO_FIELD(Int32,simId);    
    DTO_FIELD(Int32,step);  
    DTO_FIELD(Float64,currentTime);

    DTO_FIELD(Float64,transferredVolume);
    DTO_FIELD(Float64,transferredPercent);
    DTO_FIELD(Float64,remainingVolume);
    DTO_FIELD(Float64,transferRate);
    DTO_FIELD(Float64,inProgressDemandCount);

    SimMetricDto()=default;
    SimMetricDto(const SimMetricDto& that){
        stepId=that.stepId;
        simId=that.simId;
        step=that.step;
        currentTime=that.currentTime;
        transferredVolume=that.transferredVolume;
        transferredPercent=that.transferredPercent;
        remainingVolume=that.remainingVolume;
        transferRate=that.transferRate;
        inProgressDemandCount=that.inProgressDemandCount;
    }
};

class SubdomainConfigDto : public oatpp::DTO {
    DTO_INIT(SubdomainConfigDto, DTO)

    DTO_FIELD(UInt32, subdomainId);
    DTO_FIELD(UInt32, nodeCount);
    DTO_FIELD(UInt32, linkCount);
    DTO_FIELD(UInt32,gwCount);
    DTO_FIELD(UInt32,crossDomainLinkCount);
    // TODO: add routing selections once per-subdomain strategies are supported.
};

class TopologyConfigDto : public oatpp::DTO {
    DTO_INIT(TopologyConfigDto, DTO)

    DTO_FIELD(UInt32, subdomainCount);
    DTO_FIELD(List<Object<SubdomainConfigDto>>, subdomains);
};


//生成的拓扑
class DomainSpecDto : public oatpp::DTO {
  DTO_INIT(DomainSpecDto, DTO)
  DTO_FIELD(Int32, nodes);
  DTO_FIELD(Int32, edges);
};

class TopologyGenerateRequestDto : public oatpp::DTO {
  DTO_INIT(TopologyGenerateRequestDto, DTO)

  DTO_FIELD(String, filename);                       // 生成的拓扑文件名（例如 network_A.csv）
  DTO_FIELD(List<Object<DomainSpecDto>>, domains);   // 每个域的 {nodes, edges}
  DTO_FIELD(List<List<Int32>>, inter_edges);         // D×D 矩阵，i↔j 跨域边数（对称）

  // 可选参数（与前端一一对应）
  DTO_FIELD(Int32, seed)               = 42;
  DTO_FIELD(Int32, num_pairs)          = 0;
  DTO_FIELD(Float32, intra_pair_ratio) = 0.8f;
  DTO_FIELD(Int32, max_pair_hops)      = 8;
  DTO_FIELD(Int32, plot_limit)         = 1000;
  DTO_FIELD(String, out_dir)           = String("data");
};

class TopologyGenerateResponseDto : public oatpp::DTO {
  DTO_INIT(TopologyGenerateResponseDto, DTO)
  DTO_FIELD(Boolean, ok);
  DTO_FIELD(Int32, exitCode);
  DTO_FIELD(String, savedAs);  // 例如 data/network.csv
  DTO_FIELD(String, stdoutText);
  DTO_FIELD(String, stderrText);
  DTO_FIELD(String, message);  // 错误说明（如果有）
};

#include OATPP_CODEGEN_END(DTO)

#endif /* DTOs_hpp */