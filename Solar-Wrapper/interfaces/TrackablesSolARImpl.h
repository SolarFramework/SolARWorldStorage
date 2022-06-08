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

#ifndef TRACKABLES_SOLAR_IMPL_H_
#define TRACKABLES_SOLAR_IMPL_H_

#include <memory>
#include <string>
#include <Error.h>

#include <api/storage/IWorldGraphManager.h>
#include <datastructure/StorageTrackable.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <Trackable.h>
#include <TrackablesApi.h>


namespace org::openapitools::server::implem
{

/**
 * @class TrackablesSolARImpl
 * @brief implementation of TrackableAPI (class generated by OpenAPI-Generator), implements all the methods defined with the tag 'trackable' in the API specification
 *
 */

class TrackablesSolARImpl : public org::openapitools::server::api::TrackablesApi {
    public:
        explicit TrackablesSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage);
        ~TrackablesSolARImpl() override = default;


        /// @brief API method to add a trackable to the world storage. It converts the Trackable into a StorageTrackable and stores it in the worldGraph manager
        /// @param trackable: trackable to be added
        /// @param response: the response to be sent: if it succeeds, the UUID of the newly created StorageTrackable
        void add_trackable(const org::openapitools::server::model::Trackable &trackable, Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to delete a trackable, it fetches the StorageTrackable in the world storage Manager and removes it
        /// @param trackableUUID: the ID of the StorageTrackable to be removed
        /// @param response: the response to be sent: if it succeeds, a confirmation of the deletion of the StorageTrackable
        void delete_trackable(const std::string &trackableUUID, Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to get a single StorageTrackable from the world storage
        /// @param trackableUUID: the ID of the trackable to be fetched
        /// @param response: the response to be sent: if it succeeds, a JSON containing all the informations from the StorageTrackable
        void get_trackable_by_id(const std::string &trackableUUID, Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to get all the trackables currently stored in the world storage
        /// @param response: the response to be sent: if it succeeds, a JSON containing all the informations from all the StorageTrackables
        void get_trackables(Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to modify an existing trackable in the world storage.
        /// @param trackable: trackable to be modified
        /// @param response: the response to be sent: if it succeeds, the UUID of the modified StorageTrackable
        void modify_trackable(const org::openapitools::server::model::Trackable &trackable, Pistache::Http::ResponseWriter &response) override;

        /// @brief static method to convert StorageTrackable (defined by the SolAR framework) to a Trackable (defined by OpenAPI generator)
        /// @param trackable: the StorageTrackable to be converted
        /// @return the converted trackable
        static org::openapitools::server::model::Trackable from_storage(const SolAR::datastructure::StorageTrackable &trackable);

        /// @brief static method to transform a string into a vector of bytes. Used to transform a Trackable into a StorageTrackable (attribute payload)
        /// @return a vector of byte
        static std::vector<std::byte> to_bytes(const std::string &s);

        /// @brief initialize the API handler, creates the singleton m_worldStorage if it is not already done
        void init();

     private:
        /// @brief the instance of the world storage manager that will be used to handle the queries
        SRef<SolAR::api::storage::IWorldGraphManager> m_worldStorage;
};

} // namespace org::openapitools::server::api



#endif
