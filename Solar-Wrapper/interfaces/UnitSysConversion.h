/**
 * @copyright Copyright (c) 2021-2022 B-com http://www.b-com.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UNITSYSCONVERSION_H
#define UNITSYSCONVERSION_H


#include <datastructure/UnitSystem.h>
#include <Helpers.h>
#include <nlohmann/json.hpp>
#include <Trackable.h>
#include <xpcf/xpcf.h>

#include "TrackablesSolARImpl.h"

namespace xpcf = org::bcom::xpcf;
namespace org::openapitools::server::implem
{

    /// @brief method to swap between UnitSystem enums (SolAR & OPenAPI)
    /// @param the UnitSystem in openAPI format
    /// @return the UnitSystem in SolAR format
    static SolAR::datastructure::UnitSystem resolveUnitSystem(org::openapitools::server::model::UnitSystem input){
        switch(input.getValue()){
            case org::openapitools::server::model::UnitSystem::eUnitSystem::MM:
            {
                return SolAR::datastructure::UnitSystem::MM;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::CM:
            {
                return SolAR::datastructure::UnitSystem::CM;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::DM:
            {
                return SolAR::datastructure::UnitSystem::DM;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::M:
            {
                return SolAR::datastructure::UnitSystem::M;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::DAM:
            {
                return SolAR::datastructure::UnitSystem::DAM;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::HM:
            {
                return SolAR::datastructure::UnitSystem::HM;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::KM:
            {
                return SolAR::datastructure::UnitSystem::KM;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::INCH:
            {
                return SolAR::datastructure::UnitSystem::INCH;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::FOOT:
            {
                return SolAR::datastructure::UnitSystem::FOOT;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::YARD:
            {
                return SolAR::datastructure::UnitSystem::YARD;
                break;
            }
            case org::openapitools::server::model::UnitSystem::eUnitSystem::MILE:
            {
                return SolAR::datastructure::UnitSystem::MILE;
                break;
            }
            default:
            {
                return SolAR::datastructure::UnitSystem::INVALID;
            }
        }
    }

    /// @brief method to swap between UnitSystem enums (OPenAPI & SolAR)
    /// @param the UnitSystem in SolAR format
    /// @return the UnitSystem in openAPI format
    static org::openapitools::server::model::UnitSystem resolveUnitSystem(SolAR::datastructure::UnitSystem input){

        org::openapitools::server::model::UnitSystem ret;
        ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::INVALID_VALUE_OPENAPI_GENERATED);
        switch(input){
            case SolAR::datastructure::UnitSystem::MM:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::MM);
                break;
            }
            case SolAR::datastructure::UnitSystem::CM:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::CM);
                break;
            }
            case SolAR::datastructure::UnitSystem::DM:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::DM);
                break;
            }
            case SolAR::datastructure::UnitSystem::M:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::M);
                break;
            }
            case SolAR::datastructure::UnitSystem::DAM:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::DAM);
                break;
            }
            case SolAR::datastructure::UnitSystem::HM:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::HM);
                break;
            }
            case SolAR::datastructure::UnitSystem::KM:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::KM);
                break;
            }
            case SolAR::datastructure::UnitSystem::INCH:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::INCH);
                break;
            }
            case SolAR::datastructure::UnitSystem::FOOT:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::FOOT);
                break;
            }
            case SolAR::datastructure::UnitSystem::YARD:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::YARD);
                break;
            }
            case SolAR::datastructure::UnitSystem::MILE:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::MILE);
                break;
            }
            default:
            {
                ret.setValue(org::openapitools::server::model::UnitSystem::eUnitSystem::INVALID_VALUE_OPENAPI_GENERATED);
            }
        }
        return ret;
    }

}
#endif // UNITSYSCONVERSION_H
