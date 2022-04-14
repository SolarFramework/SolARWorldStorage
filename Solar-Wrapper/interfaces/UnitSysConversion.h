#ifndef UNITSYSCONVERSION_H
#define UNITSYSCONVERSION_H

#include "datastructure/UnitSystem.h"
#include "TrackablesSolARImpl.h"
#include "Trackable.h"
#include "Helpers.h"
#include "xpcf/xpcf.h"
#include <nlohmann/json.hpp>

namespace xpcf = org::bcom::xpcf;
namespace org::openapitools::server::implem
{


using namespace org::openapitools::server::model;
using namespace SolAR;

    /// @brief method to swap between UnitSystem enums (SolAR & OPenAPI)
    /// @param the UnitSystem in openAPI format
    /// @return the UnitSystem in SolAR format
    static datastructure::UnitSystem resolveUnitSystem(UnitSystem input){
        if(input.getValue() == UnitSystem::eUnitSystem::MM){return datastructure::UnitSystem::MM;}
        if(input.getValue() == UnitSystem::eUnitSystem::CM){return datastructure::UnitSystem::CM;}
        if(input.getValue() == UnitSystem::eUnitSystem::DM){return datastructure::UnitSystem::DM;}
        if(input.getValue() == UnitSystem::eUnitSystem::M){return datastructure::UnitSystem::M;}
        if(input.getValue() == UnitSystem::eUnitSystem::DAM){return datastructure::UnitSystem::DAM;}
        if(input.getValue() == UnitSystem::eUnitSystem::HM){return datastructure::UnitSystem::HM;}
        if(input.getValue() == UnitSystem::eUnitSystem::KM){return datastructure::UnitSystem::KM;}
        if(input.getValue() == UnitSystem::eUnitSystem::INCH){return datastructure::UnitSystem::INCH;}
        if(input.getValue() == UnitSystem::eUnitSystem::FOOT){return datastructure::UnitSystem::FOOT;}
        if(input.getValue() == UnitSystem::eUnitSystem::YARD){return datastructure::UnitSystem::YARD;}
        if(input.getValue() == UnitSystem::eUnitSystem::MILE){return datastructure::UnitSystem::MILE;}
        return datastructure::UnitSystem::INVALID;
    }

    /// @brief method to swap between UnitSystem enums (OPenAPI & SolAR)
    /// @param the UnitSystem in SolAR format
    /// @return the UnitSystem in openAPI format
    static UnitSystem resolveUnitSystem(datastructure::UnitSystem input){
        UnitSystem *ret = new UnitSystem();
        ret->setValue(UnitSystem::eUnitSystem::INVALID_VALUE_OPENAPI_GENERATED);
        if(input == datastructure::UnitSystem::MM){ret->setValue(UnitSystem::eUnitSystem::MM);}
        if(input == datastructure::UnitSystem::CM){ret->setValue(UnitSystem::eUnitSystem::CM);}
        if(input == datastructure::UnitSystem::DM){ret->setValue(UnitSystem::eUnitSystem::DM);}
        if(input == datastructure::UnitSystem::M){ret->setValue(UnitSystem::eUnitSystem::M);}
        if(input == datastructure::UnitSystem::DAM){ret->setValue(UnitSystem::eUnitSystem::DAM);}
        if(input == datastructure::UnitSystem::HM){ret->setValue(UnitSystem::eUnitSystem::HM);}
        if(input == datastructure::UnitSystem::KM){ret->setValue(UnitSystem::eUnitSystem::KM);}
        if(input == datastructure::UnitSystem::INCH){ret->setValue(UnitSystem::eUnitSystem::INCH);}
        if(input == datastructure::UnitSystem::FOOT){ret->setValue(UnitSystem::eUnitSystem::FOOT);}
        if(input == datastructure::UnitSystem::YARD){ret->setValue(UnitSystem::eUnitSystem::YARD);}
        if(input == datastructure::UnitSystem::MILE){ret->setValue(UnitSystem::eUnitSystem::MILE);}
        return *ret;
    }

}
#endif // UNITSYSCONVERSION_H
