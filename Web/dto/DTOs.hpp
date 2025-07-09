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
    DTO_FIELD(String, code);
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

#include OATPP_CODEGEN_END(DTO)

#endif /* DTOs_hpp */