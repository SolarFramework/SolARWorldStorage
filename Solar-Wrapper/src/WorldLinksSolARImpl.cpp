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

#include <boost/log/core.hpp>
#include <core/Log.h>
#include <Helpers.h>
#include <nlohmann/json.hpp>
#include <WorldLink.h>
#include <xpcf/core/uuid.h>
#include <xpcf/xpcf.h>

#include "TrackablesSolARImpl.h"
#include "UnitSysConversion.h"
#include "WorldAnchorsSolARImpl.h"
#include "WorldLinksSolARImpl.h"

namespace xpcf = org::bcom::xpcf;

namespace org::openapitools::server::implem {


    WorldLinksSolARImpl::WorldLinksSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage)
        : WorldLinksApi(rtr)
    {
        m_worldStorage = worldStorage;
    }

    void WorldLinksSolARImpl::add_world_link(const org::openapitools::server::model::WorldLink &worldLink, Pistache::Http::ResponseWriter &response)
    {
        //convert all the WorldLink attributes into StorageWorldLink attributes  to create one and store it in the world storage

        //authorId
        xpcf::uuids::uuid authorId = xpcf::toUUID(worldLink.getCreatorUUID());

        //transform 3d
        std::vector<float> vector = worldLink.getTransform();
        float* array = &vector[0];
        Eigen::Matrix4f matrix = Eigen::Map<Eigen::Matrix4f>(array);
        SolAR::datastructure::Transform3Df transfo(matrix);

        //world element from
        xpcf::uuids::uuid fromElementId = xpcf::toUUID(worldLink.getUUIDFrom());
        SRef<SolAR::datastructure::StorageWorldElement> fromElement;
        switch(m_worldStorage->getWorldElement(fromElementId, fromElement))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "The from element was not found in the world storage\n");
                return;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong when fetching the from element\n");
                return;
            }
        }

        //world element from
        xpcf::uuids::uuid toElementId = xpcf::toUUID(worldLink.getUUIDTo());
        SRef<SolAR::datastructure::StorageWorldElement> toElement;
        switch(m_worldStorage->getWorldElement(toElementId, toElement))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "The to element was not found in the world storage\n");
                return;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong when fetching the to element\n");
                return;
            }
        }

        //adding the link to the storage by calling the world storage method
        xpcf::uuids::uuid linkId;

        //build the worldLink
        xpcf::utils::shared_ptr<SolAR::datastructure::StorageWorldLink> storageWorldLink = xpcf::utils::make_shared<SolAR::datastructure::StorageWorldLink>(authorId, fromElement, toElement, transfo);
        switch(m_worldStorage->addWorldLink(linkId, storageWorldLink))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //initialize the json object that we will send back to the client (the WorldAnchor's id)
                std::string worldLinkIdString = xpcf::uuids::to_string(linkId);
                auto jsonObjects = nlohmann::json::array();
                nlohmann::to_json(jsonObjects, worldLinkIdString);

                //send the ID to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "The connected elements were not found in the world storage\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    void WorldLinksSolARImpl::delete_world_link(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response)
    {
        auto linkId = xpcf::toUUID(worldLinkUUID);
        switch(m_worldStorage->removeWorldLink(linkId))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
                {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Ok, "WorldLink removed\n");
                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "WorldLink not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    void WorldLinksSolARImpl::get_world_link_by_id(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response)
    {
       //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //look for the world anchor with given id
        xpcf::uuids::uuid id = xpcf::toUUID(worldLinkUUID);
        SRef<SolAR::datastructure::StorageWorldLink> storageWorldLink;

        switch(m_worldStorage->getWorldLink(id, storageWorldLink))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //StorageWorldLink found, we convert it into a WorldLink
                org::openapitools::server::model::WorldLink worldLink = from_storage(*storageWorldLink);

                //add the WorldLink to our JSON object
                to_json(jsonObjects, worldLink);

                //send the link to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());

                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "WorldLink not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    void WorldLinksSolARImpl::get_world_links(Pistache::Http::ResponseWriter &response)
    {
        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //declaration of all the objects that will be changed at each iteration of the loop
        nlohmann::json toAdd;
        org::openapitools::server::model::WorldLink worldLink;

        std::vector<SRef<SolAR::datastructure::StorageWorldLink>> vector;
        if(m_worldStorage->getWorldLinks(vector) != SolAR::FrameworkReturnCode::_SUCCESS)
        {
            //Exception raised in getWorldLinks
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong when fetching the world storage");
        }
        else
        {
            //for all the worldLinks in the worldStorage
            for(const SRef<SolAR::datastructure::StorageWorldLink> &l : vector){
                //add the current world link to the JSON object
                worldLink = from_storage(*l);
                to_json(toAdd, worldLink);
                jsonObjects.push_back(toAdd);
            }

            //send the JSON object to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void WorldLinksSolARImpl::get_attached_objects_from_uuid(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response){

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();
        nlohmann::json toAdd;

        // we get both elements attached to the worldLink
        xpcf::uuids::uuid linkId = xpcf::toUUID(worldLinkUUID);
        SRef<SolAR::datastructure::StorageWorldLink> worldLink;

        switch(m_worldStorage->getWorldLink(linkId, worldLink))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //from element
                SRef<SolAR::datastructure::StorageWorldElement> fromElement = worldLink->getFromElement();
                //since there is no type definition for worldElement in the api specifcation we have to check what kind of element it is and cast it to call the fromStorage of the corresponding class
                if (fromElement->getKind() == SolAR::datastructure::ElementKind::TRACKABLE)
                {
                    auto storageTrackable = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageTrackable>(fromElement);
                    org::openapitools::server::model::Trackable trackFrom = TrackablesSolARImpl::from_storage(*storageTrackable);
                    to_json(toAdd, trackFrom);
                    jsonObjects.push_back(toAdd);
                }
                else if(fromElement->getKind() == SolAR::datastructure::ElementKind::ANCHOR)
                {
                    auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageWorldAnchor>(fromElement);
                    org::openapitools::server::model::WorldAnchor worldAnchorFrom = WorldAnchorsSolARImpl::from_storage(*storageWorldAnchor);
                    to_json(toAdd, worldAnchorFrom);
                    jsonObjects.push_back(toAdd);
                }

                //to element
                SRef<SolAR::datastructure::StorageWorldElement> toElement = worldLink->getToElement();
                //since there is no type definition for worldElement in the api specifcation we have to check what kind of element it is and cast it to call the fromStorage of the corresponding class
                if (toElement->getKind() == SolAR::datastructure::ElementKind::TRACKABLE)
                {
                    auto storageTrackable = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageTrackable>(toElement);
                    org::openapitools::server::model::Trackable trackTo = TrackablesSolARImpl::from_storage(*storageTrackable);
                    to_json(toAdd, trackTo);
                    jsonObjects.push_back(toAdd);
                }
                else if(toElement->getKind() == SolAR::datastructure::ElementKind::ANCHOR)
                {
                    auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageWorldAnchor>(toElement);
                    org::openapitools::server::model::WorldAnchor worldAnchorTo = WorldAnchorsSolARImpl::from_storage(*storageWorldAnchor);
                    to_json(toAdd, worldAnchorTo);
                    jsonObjects.push_back(toAdd);
                }

                //send the JSON object to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "Link not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    std::pair<SRef<SolAR::datastructure::StorageWorldElement>,SRef<SolAR::datastructure::StorageWorldElement>> WorldLinksSolARImpl::get_attached_objects_from_uuid(const std::string &worldLinkUUID)
    {
        // we get both elements attached to the worldLink
        xpcf::uuids::uuid linkId = xpcf::toUUID(worldLinkUUID);
        SRef<SolAR::datastructure::StorageWorldLink> worldLink;
        SRef<SolAR::datastructure::StorageWorldElement> fromElement;
        SRef<SolAR::datastructure::StorageWorldElement> toElement;


        switch(m_worldStorage->getWorldLink(linkId, worldLink))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //from element
                fromElement = worldLink->getFromElement();


                /*
                //since there is no type definition for worldElement in the api specifcation we have to check what kind of element it is and cast it to call the fromStorage of the corresponding class
                if (fromElement->getKind() == SolAR::datastructure::ElementKind::TRACKABLE)
                {
                    auto storageTrackable = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageTrackable>(fromElement);
                }
                else if(fromElement->getKind() == SolAR::datastructure::ElementKind::ANCHOR)
                {
                    auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageWorldAnchor>(fromElement);
                    org::openapitools::server::model::WorldAnchor worldAnchorFrom = WorldAnchorsSolARImpl::from_storage(*storageWorldAnchor);
                    to_json(toAdd, worldAnchorFrom);
                    jsonObjects.push_back(toAdd);
                }*/

                //to element
                toElement = worldLink->getToElement();
                /*
                //since there is no type definition for worldElement in the api specifcation we have to check what kind of element it is and cast it to call the fromStorage of the corresponding class
                if (toElement->getKind() == SolAR::datastructure::ElementKind::TRACKABLE)
                {
                    auto storageTrackable = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageTrackable>(toElement);
                    org::openapitools::server::model::Trackable trackTo = TrackablesSolARImpl::from_storage(*storageTrackable);
                    to_json(toAdd, trackTo);
                    jsonObjects.push_back(toAdd);
                }
                else if(toElement->getKind() == SolAR::datastructure::ElementKind::ANCHOR)
                {
                    auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<SolAR::datastructure::StorageWorldAnchor>(toElement);
                    org::openapitools::server::model::WorldAnchor worldAnchorTo = WorldAnchorsSolARImpl::from_storage(*storageWorldAnchor);
                    to_json(toAdd, worldAnchorTo);
                    jsonObjects.push_back(toAdd);
                }*/

            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                break;
            }

            default :
            {
            }
        }

    }

    void WorldLinksSolARImpl::init()
    {
        try
        {
            WorldLinksApi::init();
        }
        catch (xpcf::Exception e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    org::openapitools::server::model::WorldLink WorldLinksSolARImpl::from_storage(SolAR::datastructure::StorageWorldLink worldLink)
    {
        //the object to be returned
        org::openapitools::server::model::WorldLink ret;

        //convert all the StorageWorldLink attributes into WorldLink attibutes

        //world link UUID
        std::string id = xpcf::uuids::to_string(worldLink.getId());
        ret.setUUID(id);

        //creator UUID
        std::string creatorUid = xpcf::uuids::to_string(worldLink.getAuthor());
        ret.setCreatorUUID(creatorUid);

        //element from UUID
        std::string elementTo = xpcf::uuids::to_string(worldLink.getFromElement()->getID());
        ret.setUUIDFrom(elementTo);

        //element to UUID
        std::string elementFrom = xpcf::uuids::to_string(worldLink.getToElement()->getID());
        ret.setUUIDTo(elementFrom);

        //transform
        SolAR::datastructure::Transform3Df transform3d = worldLink.getTransform();
        std::vector<float> localCRS;
        for (size_t i = 0; i < (size_t) transform3d.cols(); ++i)
           for (size_t j = 0; j < (size_t) transform3d.cols(); ++j)
           {
                localCRS.push_back(transform3d(j, i));
           }
        ret.setTransform(localCRS);

        ///======================================================================================================
        /// The following elements are totaly made up because we don't store such attributes in the world storage
        ///======================================================================================================

        //Unit system
        org::openapitools::server::model::UnitSystem unit = resolveUnitSystem(SolAR::datastructure::UnitSystem::M);
        ret.setUnit(unit);

        //Dimension (scale)
        std::vector<double> vector(3);
        for(int i = 0; i < 3; i++){
            vector[i] = 0;
        }
        ret.setLinkSize(vector);

        //keyvalue taglist (multimap to map<string,vector<string>>)
        std::map<std::string, std::vector<std::string>> tagList;
        ret.setKeyvalueTags(tagList);

        return ret;
    }
}
