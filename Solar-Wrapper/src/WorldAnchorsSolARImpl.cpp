#include "WorldAnchorsSolARImpl.h"
#include "WorldAnchor.h"
#include "Helpers.h"
#include "UnitSysConversion.h"
#include "xpcf/xpcf.h"
#include "xpcf/core/uuid.h"
#include <nlohmann/json.hpp>

namespace xpcf = org::bcom::xpcf;
namespace org {
namespace openapitools {
namespace server {
namespace implem {

    using namespace org::openapitools::server::model;
    using namespace SolAR::datastructure;
    using namespace nlohmann;
    using namespace Eigen;

    WorldAnchorsSolARImpl::WorldAnchorsSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage)
        : WorldAnchorsApi(rtr)
    {
        m_worldStorage = worldStorage;
    }

    void WorldAnchorsSolARImpl::add_world_anchor(const WorldAnchor &worldAnchor, Pistache::Http::ResponseWriter &response)
    {
        //convert all the WorldAnchor attributes into StorageWorldAnchor attributes  to create one and store it in the world storage

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(worldAnchor.getCreatorUUID());

        //localCRS
        std::vector<float> vector = worldAnchor.getLocalCRS();
        float* array = &vector[0];
        Matrix4f matrix = Map<Matrix4f>(array);
        Transform3Df localCRS(matrix);

        //unitsystem
        SolAR::datastructure::UnitSystem unitSystem = resolveUnitSystem(worldAnchor.getUnit());

        //size
        Vector3d size = Vector3d(worldAnchor.getWorldAnchorSize().data());

        //parents
        std::map<xpcf::uuids::uuid, std::pair<SRef<StorageWorldElement>, Transform3Df>> parents{};

        //children
        std::map<xpcf::uuids::uuid,SRef<StorageWorldElement>> children{};

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (std::pair<std::string,std::vector<std::string>> tag : worldAnchor.getKeyvalueTags()){
            for(std::string value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //adding the newly created StorageWorldAnchor to the worldgraph
        xpcf::uuids::uuid worldAnchorId;
        if(m_worldStorage->addWorldAnchor(worldAnchorId, creatorId, localCRS, unitSystem, size, parents, children, keyvalueTagList) != FrameworkReturnCode::_SUCCESS)
        {
            //if something went wrong
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "something went wrong when adding the anchor to the world storage");
        }

        //initialize the json object that we will send back to the client (the WorldAnchor's id)
        std::string worldAnchorIdString = xpcf::uuids::to_string(worldAnchorId);
        auto jsonObjects = nlohmann::json::array();
        to_json(jsonObjects, worldAnchorIdString);

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
            case FrameworkReturnCode::_SUCCESS :
                {
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
                response.send(Pistache::Http::Code::Ok, "WorldAnchor removed\n");
                break;
            }

            case FrameworkReturnCode::_NOT_FOUND :
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
        SRef<StorageWorldAnchor> storageWorldAnchor;

        switch(m_worldStorage->getWorldAnchor(id, storageWorldAnchor))
        {
            case FrameworkReturnCode::_SUCCESS :
            {
                //StorageWorldAnchor found, we convert it into a WorldAnchor
                WorldAnchor worldAnchor = fromStorage(*storageWorldAnchor);

                //add the WorldAnchor to our JSON object
                to_json(jsonObjects, worldAnchor);

                //send the WorldAnchor to the client
                response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
                response.send(Pistache::Http::Code::Ok, jsonObjects.dump());

                break;
            }

            case FrameworkReturnCode::_NOT_FOUND :
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
        WorldAnchor worldAnchor;

        std::vector<SRef<datastructure::StorageWorldAnchor>> vector;
        if(m_worldStorage->getWorldAnchors(vector) != FrameworkReturnCode::_SUCCESS)
        {
            //Exception raised in getWorldAnchor
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong when fetching the world storage");
        }
        else
        {
            //iteration over the content of the world storage
            for (const SRef<StorageWorldAnchor> &w : vector){
                //add the current worldAnchor to the JSON object
                worldAnchor = fromStorage(*w);
                to_json(toAdd, worldAnchor);
                jsonObjects.push_back(toAdd);
            }

            //send the JSON object to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void WorldAnchorsSolARImpl::init()
    {
        WorldAnchorsApi::init();
        try {
        }
        catch (xpcf::Exception e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    WorldAnchor WorldAnchorsSolARImpl::fromStorage(StorageWorldAnchor worldAnchor)
    {
        //the object to be returned
        WorldAnchor ret;

        //convert all the StorageWorldAnchor attributes into WorldAnchor attibutes

        //world anchor UUID
        std::string id = xpcf::uuids::to_string(worldAnchor.getID());
        ret.setUUID(id);

        //creator UUID
        std::string creatorUid = xpcf::uuids::to_string(worldAnchor.getCreatorID());
        ret.setCreatorUUID(creatorUid);

        //Transform3Df (localCRS)
        Transform3Df transform3d = worldAnchor.getLocalCrs();
        std::vector<float> localCRS;
        size_t nCols = transform3d.cols();
        for (size_t i = 0; i < nCols; ++i)
           for (size_t j = 0; j < nCols; ++j)
           {
                localCRS.push_back(transform3d(j, i));
           }
        ret.setLocalCRS(localCRS);

        //Unit system
        UnitSystem unit = resolveUnitSystem(worldAnchor.getUnitSystem());
        ret.setUnit(unit);

        //Dimension (scale)
        Vector3d dimension = worldAnchor.getSize();
        std::vector<double> vector(3);
        for(int i = 0; i < 3; i++){
            vector[i] = dimension[0,i];
        }
        ret.setWorldAnchorSize(vector);

        //keyvalue taglist (multimap to map<string,vector<string>>)
        std::map<std::string, std::vector<std::string>> tagList;
        auto storageMap = worldAnchor.getTags();
        for (auto itr = storageMap.begin() ; itr != storageMap.end(); itr++){
                if (tagList.count(itr->first) != 0){
                    std::vector<std::string>& vector = tagList.at(itr->first);
                    vector.push_back(itr->second);
                }else {
                    std::vector<std::string> vector;
                    vector.push_back(itr->second);
                    tagList.insert(std::pair<std::string, std::vector<std::string>>(itr->first, vector));
                }
        }
        ret.setKeyvalueTags(tagList);

        return ret;
    }
}
}
}
}
