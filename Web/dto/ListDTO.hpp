#ifndef LISTDTO_HPP
#define LISTDTO_HPP


#include "Web/dto/DTOs.hpp"


#include OATPP_CODEGEN_BEGIN(DTO)

template<class T>
class ListDto : public oatpp::DTO {

  DTO_INIT(ListDto, DTO)

  DTO_FIELD(UInt32, count);
  DTO_FIELD(Vector<T>, items);

};

class DemandsListDto : public ListDto<oatpp::Object<DemandDto>> {

  DTO_INIT(DemandsListDto, ListDto<oatpp::Object<DemandDto>>)

};

class SimStatusDto: public ListDto<oatpp::Object<SimResDto>> {
    DTO_INIT(SimStatusDto,ListDto<oatpp::Object<SimResDto>>)

    DTO_FIELD(UInt32,currentStep);
    DTO_FIELD(Float64,currentTime);

};



#include OATPP_CODEGEN_END(DTO)

#endif //CRUD_PAGEDTO_HPP