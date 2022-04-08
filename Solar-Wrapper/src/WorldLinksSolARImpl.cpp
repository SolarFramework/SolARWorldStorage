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

        //authorId
        xpcf::uuids::uuid authorId = xpcf::toUUID(worldLink.getCreatorUUID());

        //transform 3d
        std::vector<float> vector = worldLink.getTransform();
        float* array = &vector[0];
        Matrix4f matrix = Map<Matrix4f>(array);
        Transform3Df transfo(matrix);

        //world element from
        xpcf::uuids::uuid fromElementId = xpcf::toUUID(worldLink.getUUIDFrom());

        //world element from
        xpcf::uuids::uuid toElementId = xpcf::toUUID(worldLink.getUUIDTo());

        //adding the elements to each other
        xpcf::uuids::uuid linkId = m_worldStorage->addWorldLink(authorId, fromElementId, toElementId, transfo);


        //initialize the json object that we will send back to the client (the trackable's id)
        std::string worldLinkIdString = xpcf::uuids::to_string(linkId);
        auto jsonObjects = nlohmann::json::array();
        to_json(jsonObjects, worldLinkIdString);

        //send the ID to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        if (worldLinkIdString == "00000000-00000000-00000000-00000000"){
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong");
        }else{
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void WorldLinksSolARImpl::delete_world_link(const std::string &worldLinkUUID, Pistache::Http::ResponseWriter &response)
    {
        auto linkId = xpcf::toUUID(worldLinkUUID);
        if(m_worldStorage->removeWorldLink(linkId)){
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

        //look for the world anchor with given id
        xpcf::uuids::uuid id = xpcf::toUUID(worldLinkUUID);
        SRef<StorageWorldLink> storageWorldLink = m_worldStorage->getWorldLink(id);

        if((storageWorldLink == nullptr) || (storageWorldLink->getId() != id)){
            response.send(Pistache::Http::Code::Not_Found, "There is no world link corresponding to the given ID");
        }else {
            //StorageWorldAnchor found, we convert it into a WorldAnchor
            WorldLink worldLink = fromStorage(*storageWorldLink);

            //add the WorldAnchor to our JSON object
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

        //for all the worldLinks in the worldStorage
        for(const SRef<StorageWorldLink> &a : m_worldStorage->getWorldLinks()){
            //add the current world link to the JSON object
            worldLink = fromStorage(*a);
            to_json(toAdd, worldLink);
            jsonObjects.push_back(toAdd);
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
        auto link = m_worldStorage->getWorldLink(linkId);
        auto parentChildPair = link->getAttachedElements();

        //if they are both present we add them to the JSON
        if ( (parentChildPair.first != nullptr) && (parentChildPair.second != nullptr) )
        {
            //from element
            SRef<StorageWorldElement> fromElement = parentChildPair.first;
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
            SRef<StorageWorldElement> toElement = parentChildPair.second;
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


            //send the JSON object to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }

        //send the JSON object to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong, the elements connected to this link seem to be non existent or badly added");

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
        Transform3Df transform3d = worldLink.getTransform();
        std::vector<float> localCRS;
        size_t nCols = transform3d.cols();
        for (size_t i = 0; i < nCols; ++i)
           for (size_t j = 0; j < nCols; ++j)
           {
                localCRS.push_back(transform3d(j, i));
           }
        ret.setTransform(localCRS);

        ///======================================================================================================
        /// The following elements are totaly made up because we don't store such attributes in the world storage
        ///======================================================================================================

        //Unit system
        UnitSystem unit = resolveUnitSystem(SolAR::datastructure::UnitSystem::M);
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
}
}
}
