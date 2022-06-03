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

#include "DefaultSolARImpl.h"

namespace org::openapitools::server::implem
{

    DefaultSolARImpl::DefaultSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr)
        : DefaultApi(rtr)
    {
    }

    void DefaultSolARImpl::get_version(Pistache::Http::ResponseWriter &response){
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        //TODO put it in a variable
        response.send(Pistache::Http::Code::Ok, "Version 1.0.0");
    }

    void DefaultSolARImpl::get_ping(Pistache::Http::ResponseWriter &response) {
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        response.send(Pistache::Http::Code::Ok, "Pong");
    }

    void DefaultSolARImpl::get_admin(Pistache::Http::ResponseWriter &response) {
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        //TODO add verif on the module side
        response.send(Pistache::Http::Code::Ok, "Server running and ready !");
    }

}
