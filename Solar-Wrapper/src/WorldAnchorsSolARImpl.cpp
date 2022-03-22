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

    WorldAnchorsSolARImpl::WorldAnchorsSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr)
        : WorldAnchorsApi(rtr)
    {
    }

    void WorldAnchorsSolARImpl::add_world_anchor(const WorldAnchor &worldAnchor, Pistache::Http::ResponseWriter &response)
    {
        //convert all the WorldAnchor attributes into StorageWorldAnchor attributes  to create one and store it in the world storage

        //TODO transform 3d
        std::vector<float> vector = worldAnchor.getLocalCRS();
        float* array = &vector[0];
        Matrix4f matrix = Map<Matrix4f>(array);
        Transform3Df transfo(matrix);

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(worldAnchor.getCreatorUUID());

        //unitsystem
        SolAR::datastructure::UnitSystem unitSystem = resolveUnitSystem(worldAnchor.getUnit());

        //dimension
        Vector3d dimension = Vector3d(worldAnchor.getWorldAnchorSize().data());

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (std::pair<std::string,std::vector<std::string>> tag : worldAnchor.getKeyvalueTags()){
            for(std::string value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //adding the newly created StorageTrackable to the worldgraph
        xpcf::uuids::uuid worldAnchorId = m_worldStorage->addWorldAnchor(creatorId, transfo, unitSystem, dimension, keyvalueTagList);


        //initialize the json object that we will send back to the client (the trackable's id)
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
        ::xpcf::uuids::uuid id = xpcf::toUUID(worldAnchorUUID);
        if(m_worldStorage->removeWorldAnchor(id)){
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Ok, "WorldAnchor removed\n");
        }
        else{
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
        }
    }

    void WorldAnchorsSolARImpl::get_world_anchor_by_id(const std::string &worldAnchorUUID, Pistache::Http::ResponseWriter &response)
    {
        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //look for the world anchor with given id
        xpcf::uuids::uuid id = xpcf::toUUID(worldAnchorUUID);
        SRef<StorageWorldAnchor> storageWorldAnchor = m_worldStorage->getWorldAnchor(id);

        if((storageWorldAnchor == nullptr) || (storageWorldAnchor->getID() != id)){
            response.send(Pistache::Http::Code::Not_Found, "There is no world anchor corresponding to the given ID");
        }else {
            //StorageWorldAnchor found, we convert it into a WorldAnchor
            WorldAnchor worldAnchor = fromStorage(*storageWorldAnchor);

            //add the WorldAnchor to our JSON object
            to_json(jsonObjects, worldAnchor);

            //send the Trackable to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void WorldAnchorsSolARImpl::get_world_anchors(Pistache::Http::ResponseWriter &response)
    {

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //declaration of all the objects that will be changed at each iteration of the loop
        nlohmann::json toAdd;
        WorldAnchor worldAnchor;

        //iteration over the content of the world storage
        for (SRef<StorageWorldAnchor> a : m_worldStorage->getWorldAnchors()){
            //add the current world anchor to the JSON object
            worldAnchor = fromStorage(*a);
            to_json(toAdd, worldAnchor);

            jsonObjects.push_back(toAdd);
        }

        //send the JSON object to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void WorldAnchorsSolARImpl::init()
    {
        WorldAnchorsApi::init();
        try {
            //SolAR component initialization
            SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();
            m_worldStorage = xpcfComponentManager->resolve<SolAR::api::storage::IWorldGraphManager>();
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
        std::string creatorUid = xpcf::uuids::to_string(worldAnchor.getAuthor());
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
        Vector3d dimension = worldAnchor.getScale();
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
