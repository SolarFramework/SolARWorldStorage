/**
* World Storage API
* API ensuring interoperability between an authoring tool and a World Storage service
*
* The version of the OpenAPI document: 0.0.1
* 
*
* NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
* https://openapi-generator.tech
* Do not edit the class manually.
*/

#include "TrackablesApiImpl.h"

namespace org {
namespace openapitools {
namespace server {
namespace api {

using namespace org::openapitools::server::model;

TrackablesApiImpl::TrackablesApiImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr)
    : TrackablesApi(rtr)
{
}

void TrackablesApiImpl::add_trackable(const std::optional<Trackable> &trackable, Pistache::Http::ResponseWriter &response) {
    response.send(Pistache::Http::Code::Ok, "Do some magic\n");
}

void TrackablesApiImpl::get_trackables(Pistache::Http::ResponseWriter &response) {
    response.send(Pistache::Http::Code::Ok, "Do some magic\n");
}

void init(){
    auto worldStorage=xpcfComponentManager->resolve<SolAR::api::storage::IWorldGraphManager>();
}

}
}
}
}

