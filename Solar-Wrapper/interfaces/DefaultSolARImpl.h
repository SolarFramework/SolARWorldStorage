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

#ifndef DEFAULTSOLARIMPL_H
#define DEFAULTSOLARIMPL_H

#include <memory>
#include <Error.h>
#include <string>
#include <cstddef>
#include <DefaultApi.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>



namespace org::openapitools::server::implem
{

    using namespace org::openapitools::server::model;

    /**
     * @class
     * @brief implementation of  (class generated by OpenAPI-Generator), implements all the method defined with no tags in the API specification
     *
     */

    class DefaultSolARImpl : public org::openapitools::server::api::DefaultApi {

        public:
            explicit DefaultSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr);
            ~DefaultSolARImpl() override = default;

            void get_version(Pistache::Http::ResponseWriter &response) override;
            void get_ping(Pistache::Http::ResponseWriter &response) override;
            void get_admin(Pistache::Http::ResponseWriter &response) override;

        // DefaultApi interface
        private:
     };

}

#endif // DEFAULTSOLARIMPL_H
