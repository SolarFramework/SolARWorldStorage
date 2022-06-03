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

#include <Helpers.h>
#include <nlohmann/json.hpp>
#include <UnitSysConversion.h>
#include <WorldAnchorsSolARImpl.h>
#include <WorldAnchor.h>
#include <xpcf/xpcf.h>
#include <xpcf/core/uuid.h>

namespace org::openapitools::server::implem
{


    WorldAnchorsSolARImpl::WorldAnchorsSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage)
        : WorldAnchorsApi(rtr)
    {
        m_worldStorage = worldStorage;
    }

    void WorldAnchorsSolARImpl::add_world_anchor(const org::openapitools::server::model::WorldAnchor &worldAnchor, Pistache::Http::ResponseWriter &response)
    {
        //convert all the WorldAnchor attributes into StorageWorldAnchor attributes  to create one and store it in the world storage

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(worldAnchor.getCreatorUUID());

        //localCRS
        std::vector<float> vector = worldAnchor.getLocalCRS();
        float* array = &vector[0];
        Eigen::Matrix4f matrix = Eigen::Map<Eigen::Matrix4f>(array);
        SolAR::datastructure::Transform3Df localCRS(matrix);

        //unitsystem
        SolAR::datastructure::UnitSystem unitSystem = resolveUnitSystem(worldAnchor.getUnit());

        //size
        Eigen::Vector3d size = Eigen::Vector3d(worldAnchor.getWorldAnchorSize().data());

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (std::pair<std::string,std::vector<std::string>> tag : worldAnchor.getKeyvalueTags()){
            for(std::string value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //name
        std::string name = worldAnchor.getName();

        //create a world anchor
        SRef<SolAR::datastructure::StorageWorldAnchor> storageWorldAnchor = xpcf::utils::make_shared<SolAR::datastructure::StorageWorldAnchor>(creatorId, localCRS, unitSystem, size, keyvalueTagList, name);

        //adding the newly created StorageWorldAnchor to the worldgraph
        xpcf::uuids::uuid worldAnchorId;
        if(m_worldStorage->addWorldAnchor(worldAnchorId, storageWorldAnchor) != SolAR::FrameworkReturnCode::_SUCCESS)
        {
            //if something went wrong
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "something went wrong when adding the anchor to the world storage");
            return;
        }

        //initialize the json object that we will send back to the client (the WorldAnchor's id)
        std::string worldAnchorIdString = xpcf::uuids::to_string(worldAnchorId);
        auto jsonObjects = nlohmann::json::array();
        nlohmann::to_json(jsonObjects, worldAnchorIdString);

        //send the ID to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void WorldAnchorsSolARImpl::delete_world_anchor(const std::string &worldAnchorUUID, Pistache::Http::ResponseWriter &response)
    {
        //world anchor uuid
        xpcf::uuids::uuid id = xpcf::toUUID(worldAnchorUUID);
        switch(m_worldStorage->removeWorldAnchor(id))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
                {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Ok, "WorldAnchor removed\n");
                break;
            }

        case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "WorldAnchor not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    void WorldAnchorsSolARImpl::get_world_anchor_by_id(const std::string &worldAnchorUUID, Pistache::Http::ResponseWriter &response)
    {
        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //look for the world anchor with given id
        xpcf::uuids::uuid id = xpcf::toUUID(worldAnchorUUID);
        SRef<SolAR::datastructure::StorageWorldAnchor> storageWorldAnchor;

        switch(m_worldStorage->getWorldAnchor(id, storageWorldAnchor))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //StorageWorldAnchor found, we convert it into a WorldAnchor
                org::openapitools::server::model::WorldAnchor worldAnchor = from_storage(*storageWorldAnchor);

                //add the WorldAnchor to our JSON object
                to_json(jsonObjects, worldAnchor);

                //send the WorldAnchor to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());

                break;
            }

        case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "WorldAnchor not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    void WorldAnchorsSolARImpl::get_world_anchors(Pistache::Http::ResponseWriter &response)
    {
        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //declaration of all the objects that will be changed at each iteration of the loop
        nlohmann::json toAdd;
        org::openapitools::server::model::WorldAnchor worldAnchor;

        std::vector<SRef<SolAR::datastructure::StorageWorldAnchor>> vector;
        if(m_worldStorage->getWorldAnchors(vector) != SolAR::FrameworkReturnCode::_SUCCESS)
        {
            //Exception raised in getWorldAnchor
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong when fetching the world storage");
        }
        else
        {
            //iteration over the content of the world storage
            for (const SRef<SolAR::datastructure::StorageWorldAnchor> &w : vector){
                //add the current worldAnchor to the JSON object
                worldAnchor = from_storage(*w);
                to_json(toAdd, worldAnchor);
                jsonObjects.push_back(toAdd);
            }

            //send the JSON object to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void WorldAnchorsSolARImpl::modify_world_anchor(const model::WorldAnchor &worldAnchor, Pistache::Http::ResponseWriter &response)
    {

    }

    void WorldAnchorsSolARImpl::init()
    {
        try
        {
            WorldAnchorsApi::init();
        }
        catch (xpcf::Exception e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    org::openapitools::server::model::WorldAnchor WorldAnchorsSolARImpl::from_storage(const SolAR::datastructure::StorageWorldAnchor &worldAnchor)
    {
        //the object to be returned
        org::openapitools::server::model::WorldAnchor ret;

        //convert all the StorageWorldAnchor attributes into WorldAnchor attibutes

        //world anchor UUID
        std::string id = xpcf::uuids::to_string(worldAnchor.getID());
        ret.setUUID(id);

        //name
        ret.setName(worldAnchor.getName());

        //creator UUID
        std::string creatorUid = xpcf::uuids::to_string(worldAnchor.getCreatorID());
        ret.setCreatorUUID(creatorUid);

        //Transform3Df (localCRS)
        SolAR::datastructure::Transform3Df transform3d = worldAnchor.getLocalCrs();
        std::vector<float> localCRS;
        for (size_t i = 0; i < (size_t) transform3d.cols(); ++i)
        {
            for (size_t j = 0; j < (size_t) transform3d.rows(); ++j)
            {
                localCRS.push_back(transform3d(j, i));
            }
        }
        ret.setLocalCRS(localCRS);

        //Unit system
        org::openapitools::server::model::UnitSystem unit = resolveUnitSystem(worldAnchor.getUnitSystem());
        ret.setUnit(unit);

        //Dimension (scale)
        Eigen::Vector3d dimension = worldAnchor.getSize();
        std::vector<double> vector(3);
        for(int i = 0; i < 3; i++){
            vector[i] = dimension[0,i];
        }
        ret.setWorldAnchorSize(vector);

        //keyvalue taglist (multimap to map<string,vector<string>>)
        std::map<std::string, std::vector<std::string>> tagList;
        auto storageMap = worldAnchor.getTags();
        for (const auto &tag : storageMap){
                if (tagList.count(tag.first) != 0){
                    std::vector<std::string>& vector = tagList.at(tag.first);
                    vector.push_back(tag.second);
                }else {
                    std::vector<std::string> vector;
                    vector.push_back(tag.second);
                    tagList.insert(std::pair<std::string, std::vector<std::string>>(tag.first, vector));
                }
        }
        ret.setKeyvalueTags(tagList);

        return ret;
    }
}
