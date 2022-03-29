/*
* TrackablesSolARImpl.h
*
* 
*/

#ifndef TRACKABLES_SOLAR_IMPL_H_
#define TRACKABLES_SOLAR_IMPL_H_


#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <memory>
#include <optional>

#include <TrackablesApi.h>


#include <Error.h>
#include <Trackable.h>
#include <datastructure/StorageTrackable.h>
#include "api/storage/IWorldGraphManager.h"
#include <string>

namespace org::openapitools::server::implem
{

using namespace org::openapitools::server::model;


/**
 * @class TrackablesSolARImpl
 * @brief implementation of TrackableAPI (class generated by OpenAPI-Generator), implements all the method defined with the tag 'trackable' in the API specification
 *
 */

class TrackablesSolARImpl : public org::openapitools::server::api::TrackablesApi {
    public:
        explicit TrackablesSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage);
        ~TrackablesSolARImpl() override = default;


        /// @brief API method to add a trackable to the world storage. It converts the Trackable into a StorageTrackable and stores it in the worldGraph manager
        /// @param trackable : trackable to be added
        /// @param response : the response to be sent : if it succeeds, the UUID of the newly created StorageTrackable
        void add_trackable(const Trackable &trackable, Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to delete a trackable, it fetches the StorageTrackable in the world storage Manager and removes it
        /// @param trackableUUID : the ID of the StorageTrackable to be removed
        /// @param response : the response to be sent : if it succeeds, a confirmation of the deletion of the StorageTrackable
        void delete_trackable(const std::string &trackableUUID, Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to get a single StorageTrackable from the world storage
        /// @param trackableUUID : the ID of the trackable to be fetched
        /// @param response : the response to be sent : if it succeeds, a JSON containing all the informations from the StorageTrackable
        void get_trackable_by_id(const std::string &trackableUUID, Pistache::Http::ResponseWriter &response) override;

        /// @brief API method to get all the trackables currently stored in the world storage
        /// @param response : the response to be sent : if it succeeds, a JSON containing all the informations from all the StorageTrackables
        void get_trackables(Pistache::Http::ResponseWriter &response) override;

        /// @brief static method to convert StorageTrackable (defined by the SolAR framework) to a Trackable (defined by OpenAPI generator)
        /// @param trackable : the StorageTrackable to be converted
        /// @return the converted trackable
        static Trackable fromStorage(SolAR::datastructure::StorageTrackable trackable);

        /// @brief static method to transform a string into a vector of byte. Used to transform a Trackable into a StorageTrackable (attribute payload)
        /// @return a vector of byte
        static std::vector<std::byte> toBytes(std::string const& s);

        /// @brief initialize the API handler, creates the singleton m_worldStorage if it is not already done
        void init();

     private:
        /// @brief the instance of the world storage manager that will be used to handle the queries
        SRef<SolAR::api::storage::IWorldGraphManager> m_worldStorage;
};

} // namespace org::openapitools::server::api



#endif
