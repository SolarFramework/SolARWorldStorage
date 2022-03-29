#include "WorldLinksSolARImpl.h"
#include "TrackablesSolARImpl.h"
#include "WorldAnchorsSolARImpl.h"
#include "WorldLink.h"
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

    WorldLinksSolARImpl::WorldLinksSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr, SRef<SolAR::api::storage::IWorldGraphManager> worldStorage)
        : WorldLinksApi(rtr)
    {
        m_worldStorage = worldStorage;
    }

    void WorldLinksSolARImpl::add_world_link(const WorldLink &worldLink, Pistache::Http::ResponseWriter &response)
    {
        //convert all the WorldLink attributes into StorageWorldLink attributes  to create one and store it in the world storage

        //transform 3d
        std::vector<float> vector = worldLink.getTransform();
        float* array = &vector[0];
        Matrix4f matrix = Map<Matrix4f>(array);
        Transform3Df transfo(matrix);

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(worldLink.getCreatorUUID());

        //world element from - uuid
        xpcf::uuids::uuid fromElement = xpcf::toUUID(worldLink.getUUIDFrom());

        //world element from - uuid
        xpcf::uuids::uuid toElement = xpcf::toUUID(worldLink.getUUIDTo());

        //unitsystem
        SolAR::datastructure::UnitSystem unitSystem = resolveUnitSystem(worldLink.getUnit());

        //dimension
        Vector3d dimension = Vector3d(worldLink.getLinkSize().data());

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (std::pair<std::string,std::vector<std::string>> tag : worldLink.getKeyvalueTags()){
            for(std::string value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //adding the newly created StorageWordLink to the worldgraph
        xpcf::uuids::uuid worldLinkId = m_worldStorage->addWorldLink(creatorId, fromElement, toElement, transfo, unitSystem, dimension, keyvalueTagList);


        //initialize the json object that we will send back to the client (the trackable's id)
        std::string worldLinkIdString = xpcf::uuids::to_string(worldLinkId);
        auto jsonObjects = nlohmann::json::array();
        to_json(jsonObjects, worldLinkIdString);

        //send the ID to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void WorldLinksSolARImpl::delete_world_link(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response)
    {
        //world link uuid
        ::xpcf::uuids::uuid id = xpcf::toUUID(worldLinkUUID);
        if(m_worldStorage->removeWorldLink(id)){
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Ok, "WorldLink removed\n");
        }
        else{
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
        }
    }

    void WorldLinksSolARImpl::get_world_link_by_id(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response)
    {
        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //look for the world link with given id
        xpcf::uuids::uuid id = xpcf::toUUID(worldLinkUUID);
        SRef<StorageWorldLink> storageWorldLink = m_worldStorage->getWorldLink(id);

        if((storageWorldLink == nullptr) || (storageWorldLink->getID() != id)){
            response.send(Pistache::Http::Code::Not_Found, "There is no world link corresponding to the given ID");
        }else {
            //StorageWorldLink found, we convert it into a WorldLink
            WorldLink worldLink = fromStorage(*storageWorldLink);

            //add the WorldLink to our JSON object
            to_json(jsonObjects, worldLink);

            //send the Trackable to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void WorldLinksSolARImpl::get_world_links(Pistache::Http::ResponseWriter &response)
    {

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //declaration of all the objects that will be changed at each iteration of the loop
        nlohmann::json toAdd;
        WorldLink worldLink;

        //iteration over the content of the world storage
        for (SRef<StorageWorldElement> a : m_worldStorage->getWorldElements()){


            ///TEST
            if (a->isTrackable())
            {
                auto storageTrackable = xpcf::utils::dynamic_pointer_cast<datastructure::StorageTrackable>(a);
                Trackable trackFrom = TrackablesSolARImpl::fromStorage(*storageTrackable);
                to_json(toAdd, trackFrom);
                jsonObjects.push_back(toAdd);
            }
            else if(a->isWorldAnchor())
            {
                auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<datastructure::StorageWorldAnchor>(a);
                WorldAnchor worldAnchorFrom = WorldAnchorsSolARImpl::fromStorage(*storageWorldAnchor);
                to_json(toAdd, worldAnchorFrom);
                jsonObjects.push_back(toAdd);

            }
            ///FINTEST


            /*
            //add the current world link to the JSON object
            worldLink = fromStorage(*a);
            to_json(toAdd, worldLink);
            jsonObjects.push_back(toAdd);
            */
        }

        //send the JSON object to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void WorldLinksSolARImpl::get_attached_objects_from_uuid(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response){

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();
        nlohmann::json toAdd;

        // we get both elements attached to the worldLink
        xpcf::uuids::uuid linkId = xpcf::toUUID(worldLinkUUID);
        std::vector<SRef<StorageWorldElement>> attachedElements = m_worldStorage->getConnectedElements(linkId);
        if (attachedElements.size() == 2)
        {
            //from element
            SRef<StorageWorldElement> fromElement = attachedElements[0];
            //since there is no type definition for worldElement in the api specifcation we have to check what kind of element it is and cast it to call the fromStorage of the corresponding class
            if (fromElement->isTrackable())
            {
                auto storageTrackable = xpcf::utils::dynamic_pointer_cast<datastructure::StorageTrackable>(fromElement);
                Trackable trackFrom = TrackablesSolARImpl::fromStorage(*storageTrackable);
                to_json(toAdd, trackFrom);
                jsonObjects.push_back(toAdd);
            }
            else if(fromElement->isWorldAnchor())
            {
                auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<datastructure::StorageWorldAnchor>(fromElement);
                WorldAnchor worldAnchorFrom = WorldAnchorsSolARImpl::fromStorage(*storageWorldAnchor);
                to_json(toAdd, worldAnchorFrom);
                jsonObjects.push_back(toAdd);

            }

            //to element
            SRef<StorageWorldElement> toElement = attachedElements[1];
            //since there is no type definition for worldElement in the api specifcation we have to check what kind of element it is and cast it to call the fromStorage of the corresponding class
            if (toElement->isTrackable())
            {
                auto storageTrackable = xpcf::utils::dynamic_pointer_cast<datastructure::StorageTrackable>(toElement);
                Trackable trackFrom = TrackablesSolARImpl::fromStorage(*storageTrackable);
                to_json(toAdd, trackFrom);
                jsonObjects.push_back(toAdd);
            }
            else if(toElement->isWorldAnchor())
            {
                auto storageWorldAnchor = xpcf::utils::dynamic_pointer_cast<datastructure::StorageWorldAnchor>(toElement);
                WorldAnchor worldAnchorFrom = WorldAnchorsSolARImpl::fromStorage(*storageWorldAnchor);
                to_json(toAdd, worldAnchorFrom);
                jsonObjects.push_back(toAdd);


            }
        }
        //if there is only one connected elt (conected to non existent elts)
        else if (attachedElements.size() == 1)
        {
            response.send(Pistache::Http::Code::Not_Found, "The World Link is connected to non-existent world elements");
        }
        //if there are no connected elements (because the link doesn't exists)
        else
        {
            response.send(Pistache::Http::Code::Not_Found, "There is no world link corresponding to the given ID");
        }

        //send the JSON object to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());

    }

    void WorldLinksSolARImpl::init()
    {
        WorldLinksApi::init();
        try {
        }
        catch (xpcf::Exception e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    WorldLink WorldLinksSolARImpl::fromStorage(StorageWorldLink worldLink)
    {
        //the object to be returned
        WorldLink ret;

        //convert all the StorageWorldLink attributes into WorldLink attibutes

        //world link UUID
        std::string id = xpcf::uuids::to_string(worldLink.getID());
        ret.setUUID(id);

        //creator UUID
        std::string creatorUid = xpcf::uuids::to_string(worldLink.getAuthor());
        ret.setCreatorUUID(creatorUid);

        //element from UUID
        std::string elementTo = xpcf::uuids::to_string(worldLink.getFromElement());
        ret.setUUIDFrom(elementTo);

        //element to UUID
        std::string elementFrom = xpcf::uuids::to_string(worldLink.getToElement());
        ret.setUUIDTo(elementFrom);

        //Transform3Df (localCRS)
        Transform3Df transform3d = worldLink.getTransform();
        std::vector<float> localCRS;
        size_t nCols = transform3d.cols();
        for (size_t i = 0; i < nCols; ++i)
           for (size_t j = 0; j < nCols; ++j)
           {
                localCRS.push_back(transform3d(j, i));
           }
        ret.setTransform(localCRS);

        //Unit system
        UnitSystem unit = resolveUnitSystem(worldLink.getUnitSystem());
        ret.setUnit(unit);

        //Dimension (scale)
        Vector3d dimension = worldLink.getScale();
        std::vector<double> vector(3);
        for(int i = 0; i < 3; i++){
            vector[i] = dimension[0,i];
        }
        ret.setLinkSize(vector);

        //keyvalue taglist (multimap to map<string,vector<string>>)
        std::map<std::string, std::vector<std::string>> tagList;
        auto storageMap = worldLink.getTags();
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
