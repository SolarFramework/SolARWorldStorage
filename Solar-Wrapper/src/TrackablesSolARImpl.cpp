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

#include <Eigen/Eigen>
#include <Helpers.h>
#include <nlohmann/json.hpp>
#include <TrackablesSolARImpl.h>
#include <Trackable.h>
#include <UnitSysConversion.h>
#include <xpcf/xpcf.h>
#include <xpcf/core/uuid.h>

namespace xpcf = org::bcom::xpcf;

namespace org::openapitools::server::implem
{


    TrackablesSolARImpl::TrackablesSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage)
        : TrackablesApi(rtr)
    {
        m_worldStorage = worldStorage;
    }

    void TrackablesSolARImpl::add_trackable(const org::openapitools::server::model::Trackable &trackable, Pistache::Http::ResponseWriter &response)
    {
        //convert all the Trackable attributes into StorageTrackable attributes  to create one and store it in the world storage

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(trackable.getCreatorUUID());

        //localCRS
        std::vector<float> vector = trackable.getLocalCRS();
        Eigen::Matrix4f matrix = Eigen::Map<Eigen::Matrix<float,4,4,Eigen::RowMajor>>(vector.data());
        SolAR::datastructure::Transform3Df localCRS(matrix);

        //unitsystem
        SolAR::datastructure::UnitSystem unitSystem = resolveUnitSystem(trackable.getUnit());

        //dimension
        Eigen::Vector3d dimension = Eigen::Vector3d(trackable.getTrackableSize().data());

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (const auto &tag : trackable.getKeyvalueTags()){
            for(const auto &value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //trackable type
        SolAR::datastructure::StorageTrackableType type = SolAR::datastructure::resolveTrackableType(trackable.getTrackableType());

        //encoding info
        SolAR::datastructure::EncodingInfo encodingInfo(trackable.getTrackableEncodingInformation().getDataFormat(), trackable.getTrackableEncodingInformation().getVersion());

        //payload
        std::vector<std::byte> payload = TrackablesSolARImpl::to_bytes(trackable.getTrackablePayload());

        //name
        std::string name = trackable.getName();

        //create the trackable
        SRef<SolAR::datastructure::StorageTrackable> storageTrackable = xpcf::utils::make_shared<SolAR::datastructure::StorageTrackable>(creatorId, localCRS, unitSystem, dimension, keyvalueTagList, type, encodingInfo, payload, name);

        //adding the newly created StorageTrackable to the worldgraph
        xpcf::uuids::uuid trackableId;
        if(m_worldStorage->addTrackable(trackableId, storageTrackable) != SolAR::FrameworkReturnCode::_SUCCESS)
        {
            //if something went wrong
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "something went wrong when adding the trackable to the world storage");
            return;
        }

        //initialize the json object that we will send back to the client (the trackable's id)
        std::string trackableIdString = xpcf::uuids::to_string(trackableId);
        auto jsonObjects = nlohmann::json::array();
        nlohmann::to_json(jsonObjects, trackableIdString);

        //send the ID to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void TrackablesSolARImpl::delete_trackable(const std::string &trackableUUID, Pistache::Http::ResponseWriter &response)
    {
        //trackable uuid
        xpcf::uuids::uuid id = xpcf::toUUID(trackableUUID);
        switch(m_worldStorage->removeTrackable(id))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Ok, "Trackable removed\n");
                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "Trackable not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }
    }

    void TrackablesSolARImpl::get_trackable_by_id(const std::string &trackableUUID, Pistache::Http::ResponseWriter &response)
    {
        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //look for the trackable with given id
        xpcf::uuids::uuid id = xpcf::toUUID(trackableUUID);
        SRef<SolAR::datastructure::StorageTrackable> storageTrackable;

        switch(m_worldStorage->getTrackable(id, storageTrackable))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //StorageTrackable found, we convert it into a Trackable
                org::openapitools::server::model::Trackable trackable = from_storage(*storageTrackable);

                //add the Trackable to our JSON object
                to_json(jsonObjects, trackable);

                //send the Trackable to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());

                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "Trackable not found\n");
                break;
            }

            default :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
            }
        }


    }

    void TrackablesSolARImpl::get_trackables(Pistache::Http::ResponseWriter &response)
    {

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //declaration of all the objects that will be changed at each iteration of the loop
        nlohmann::json toAdd;
        org::openapitools::server::model::Trackable track;

        std::vector<SRef<SolAR::datastructure::StorageTrackable>> vector;
        if(m_worldStorage->getTrackables(vector) != SolAR::FrameworkReturnCode::_SUCCESS)
        {
            //Exception raised in getTrackables
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong when fetching the trackables from the world storage");
        }
        else
        {
            //iteration over the content of the world storage
            for (const SRef<SolAR::datastructure::StorageTrackable> &t : vector){
                //add the current trackable to the JSON object
                track = from_storage(*t);
                to_json(toAdd, track);
                jsonObjects.push_back(toAdd);
            }

            //send the JSON object to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void TrackablesSolARImpl::modify_trackable(const model::Trackable &trackable, Pistache::Http::ResponseWriter &response)
    {
        //convert all the Trackable attributes into StorageTrackable attributes  to create one and store it in the world storage

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(trackable.getCreatorUUID());

        //localCRS
        std::vector<float> vector = trackable.getLocalCRS();
        Eigen::Matrix4f matrix = Eigen::Map<Eigen::Matrix<float,4,4,Eigen::RowMajor>>(vector.data());
        SolAR::datastructure::Transform3Df localCRS(matrix);

        //unitsystem
        SolAR::datastructure::UnitSystem unitSystem = resolveUnitSystem(trackable.getUnit());

        //dimension
        Eigen::Vector3d dimension = Eigen::Vector3d(trackable.getTrackableSize().data());

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (const auto &tag : trackable.getKeyvalueTags()){
            for(const auto &value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //trackable type
        SolAR::datastructure::StorageTrackableType type = SolAR::datastructure::resolveTrackableType(trackable.getTrackableType());

        //encoding info
        SolAR::datastructure::EncodingInfo encodingInfo(trackable.getTrackableEncodingInformation().getDataFormat(), trackable.getTrackableEncodingInformation().getVersion());

        //payload
        std::vector<std::byte> payload = TrackablesSolARImpl::to_bytes(trackable.getTrackablePayload());

        //name
        std::string name = trackable.getName();

        //id
        boost::uuids::uuid id = xpcf::toUUID(trackable.getUUID());

        //create the trackable
        SRef<SolAR::datastructure::StorageTrackable> storageTrackable = xpcf::utils::make_shared<SolAR::datastructure::StorageTrackable>(id, creatorId, localCRS, unitSystem, dimension, keyvalueTagList, type, encodingInfo, payload, name);

        //adding the newly created StorageTrackable to the worldgraph
        xpcf::uuids::uuid trackableId;
        switch(m_worldStorage->modifyTrackable(trackableId, storageTrackable))
        {
            case SolAR::FrameworkReturnCode::_SUCCESS :
            {
                //initialize the json object that we will send back to the client (the trackable's id)
                std::string trackableIdString = xpcf::uuids::to_string(trackableId);
                auto jsonObjects = nlohmann::json::array();
                nlohmann::to_json(jsonObjects, trackableIdString);

                //send the ID to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());

                break;
            }

            case SolAR::FrameworkReturnCode::_NOT_FOUND :
            {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Not_Found, "Trackable not found\n");

                break;
            }

            default :
            {
            //if something went wrong
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "something went wrong when modifying the trackable to the world storage");

            }
        }

        return;
    }

    org::openapitools::server::model::Trackable TrackablesSolARImpl::from_storage(const SolAR::datastructure::StorageTrackable &trackable){
        //the object to be returned
        org::openapitools::server::model::Trackable ret;

        //convert all the StorageTrackable attributes into Trackable attibutes

        //trackable UUID
        std::string id = xpcf::uuids::to_string(trackable.getID());
        ret.setUUID(id);

        //creator UUID
        std::string creatorUid = xpcf::uuids::to_string(trackable.getCreatorID());
        ret.setCreatorUUID(creatorUid);

        //name
        ret.setName(trackable.getName());

        //Trackable type
        std::string type = resolveTrackableType(trackable.getType());
        ret.setTrackableType(type);

        //EncodingInfo struct
        org::openapitools::server::model::EncodingInformationStructure encodingInfo;
        SolAR::datastructure::EncodingInfo storageEncodingInfo(trackable.getEncodingInfo());
        encodingInfo.setDataFormat(storageEncodingInfo.getDataFormat());
        encodingInfo.setVersion(storageEncodingInfo.getVersion());
        ret.setTrackableEncodingInformation(encodingInfo);

        //Payload
        std::vector<std::byte> storagePayload = trackable.getPayload();
        std::string payload(reinterpret_cast<const char *>(&storagePayload[0]), storagePayload.size());
        ret.setTrackablePayload(payload);

        //Transform3Df (localCRS)
        SolAR::datastructure::Transform3Df transform3d = trackable.getLocalCrs();
        Eigen::Matrix4f matrix = transform3d.matrix();
        std::vector<float> localCRS;
        for (size_t i = 0; i < (size_t) matrix.rows(); i++)
        {
            for (size_t j = 0; j < (size_t) matrix.cols(); j++)
            {
                localCRS.push_back(matrix(i, j));
            }
        }
        ret.setLocalCRS(localCRS);

        //Unit system
        org::openapitools::server::model::UnitSystem unit = resolveUnitSystem(trackable.getUnitSystem());
        ret.setUnit(unit);

        //Dimension (scale)
        Eigen::Vector3d dimension = trackable.getSize();
        std::vector<double> vector(3);
        for(int i = 0; i < 3; i++)
        {
            vector[i] = dimension[0,i];
        }
        ret.setTrackableSize(vector);

        //keyvalue taglist (multimap to map<string,vector<string>>)
        std::map<std::string, std::vector<std::string>> tagList;
        auto storageMap = trackable.getTags();
        for (const auto &tag : storageMap)
        {
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

    std::vector<std::byte> TrackablesSolARImpl::to_bytes(const std::string &s)
    {
        std::vector<std::byte> bytes;
        bytes.reserve(std::size(s));
        std::transform(std::begin(s), std::end(s), std::back_inserter(bytes), [](char c){
            return std::byte(c);
        });

        return bytes;
    }

    void TrackablesSolARImpl::init(){
        try {
            TrackablesApi::init();
        }
        catch (xpcf::Exception e)
        {
            std::cout << e.what() << std::endl;
        }
    }

}

