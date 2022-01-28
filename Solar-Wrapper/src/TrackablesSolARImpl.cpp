#include "TrackablesSolARImpl.h"
#include "Trackable.h"
#include "Helpers.h"
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

    TrackablesSolARImpl::TrackablesSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr)
        : TrackablesApi(rtr)
    {
    }

    void TrackablesSolARImpl::add_trackable(const Trackable &trackable, Pistache::Http::ResponseWriter &response){

        //convert all the Trackable attributes into StorageTrackable attributes  to create one and store it in the world storage

        //TODO transform 3d
        std::vector<float> vector = trackable.getLocalCRS();
        float* array = &vector[0];// = trackable.getLocalCRS();
        Matrix4f matrix = Map<Matrix4f>(array);
        Transform3Df transfo(matrix);

        //creator uuid
        xpcf::uuids::uuid creatorId = xpcf::toUUID(trackable.getCreatorUID());

        //trackable type
        StorageTrackableType type = resolveTrackableType(trackable.getTrackableType());

        //encoding info
        EncodingInfo encodingInfo(trackable.getTrackableEncodingInformation().getDataFormat(), trackable.getTrackableEncodingInformation().getVersion());

        //payload
        std::vector<std::byte> payload = TrackablesSolARImpl::toBytes(trackable.getTrackablePayload());


        //unitsystem
        UnitSystem unitSystem = resolveUnitSystem(trackable.getUnitSystem());

        //dimension
        Vector3d dimension = Vector3d(trackable.getTrackableDimension().data());

        //taglist
        std::multimap<std::string,std::string> keyvalueTagList;
        for (std::pair<std::string,std::vector<std::string>> tag : trackable.getKeyvalueTagList()){
            for(std::string value : tag.second){
                keyvalueTagList.insert({tag.first,value});
            }
        }

        //adding the newly created StorageTrackable to the worldgraph
        xpcf::uuids::uuid trackableId = m_worldStorage->addTrackable(creatorId,type, encodingInfo, payload, transfo, unitSystem, dimension, keyvalueTagList);


        //initialize the json object that we will send back to the client (the trackable's id)
        std::string trackableIdString = xpcf::uuids::to_string(trackableId);
        auto jsonObjects = nlohmann::json::array();
        to_json(jsonObjects, trackableIdString);

        //send the ID to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void TrackablesSolARImpl::get_trackables(Pistache::Http::ResponseWriter &response) {

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //declaration of all the objects that will be changed at each iteration of the loop
        nlohmann::json toAdd;
        std::string trackableId;
        Trackable track;

        //iteration over the content of the world storage
        for (SRef<StorageTrackable> t : m_worldStorage->getTrackables()){
            //add the current trackable to the JSON object
            track = fromStorage(*t);
            to_json(toAdd, track);

            //also add its ID since the Trackable object (specified by Open Api generator) does not have an ID
            trackableId = xpcf::uuids::to_string(t->getID());
            toAdd["id"]= trackableId;

            jsonObjects.push_back(toAdd);
        }

        //send the JSON object to the client
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
    }

    void TrackablesSolARImpl::delete_trackable(const std::string &trackableId, Pistache::Http::ResponseWriter &response) {
        //trackable uuid
        ::xpcf::uuids::uuid id = xpcf::toUUID(trackableId);
        if(m_worldStorage->removeTrackable(id)){
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Ok, "Trackable removed\n");
        }
        else{
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
            response.send(Pistache::Http::Code::Internal_Server_Error, "Something went wrong\n");
        }
    }

    void TrackablesSolARImpl::get_trackable_by_id(const std::string &trackableId, Pistache::Http::ResponseWriter &response) {

        //initialize the json object that we will send back to the client
        auto jsonObjects = nlohmann::json::array();

        //look for the trackable with given id
        xpcf::uuids::uuid id = xpcf::toUUID(trackableId);
        SRef<StorageTrackable> storageTrackable = m_worldStorage->getTrackable(id);

        if(id != storageTrackable->getID()){
            //if it's not found the world storage manager will send back a StorageTrackable with ID 00000000-0000-0000-0000-000000000000
            response.send(Pistache::Http::Code::Not_Found, "Le trackable avec l'ID donnée n'existe pas");
        }else {
            //StorageTrackable found, we convert it into a Trackable
            Trackable trackable = fromStorage(*storageTrackable);

            //add the Trackable to our JSON object
            to_json(jsonObjects, trackable);

            //also add its ID since the Trackable object (specified by Open Api generator) does not have an ID
            jsonObjects["id"]= trackableId;

            //send the Trackable to the client
            response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
            response.send(Pistache::Http::Code::Ok, jsonObjects.dump());
        }
    }

    void TrackablesSolARImpl::init(){
        TrackablesApi::init();
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

    Trackable TrackablesSolARImpl::fromStorage(StorageTrackable trackable){
        //the object to be returned
        Trackable ret;

        //convert all the StorageTrackable attributes into Trackable attibutes

        //creator UUID
        std::string creatorUid = xpcf::uuids::to_string(trackable.getAuthor());
        ret.setCreatorUID(creatorUid);

        //Trackable type
        std::string type = resolveTrackableType(trackable.getType());
        ret.setTrackableType(type);

        //EncodingInfo struct
        EncodingInformationStructure encodingInfo;
        EncodingInfo storageEncodingInfo(trackable.getEncodingInfo());
        encodingInfo.setDataFormat(storageEncodingInfo.getDataFormat());
        encodingInfo.setVersion(storageEncodingInfo.getVersion());
        ret.setTrackableEncodingInformation(encodingInfo);

        //Payload
        std::vector<std::byte> storagePayload = trackable.getPayload();
        std::string payload(reinterpret_cast<const char *>(&storagePayload[0]), storagePayload.size());
        ret.setTrackablePayload(payload);

        //Transform3Df (localCRS)
        Transform3Df transform3d = trackable.getLocalCrs();
        std::vector<float> localCRS;
        size_t nCols = transform3d.cols();
        for (size_t i = 0; i < nCols; ++i)
           for (size_t j = 0; j < nCols; ++j)
           {
                localCRS.push_back(transform3d(j, i));
           }
        ret.setLocalCRS(localCRS);

        //Unit system
        std::string unit = resolveUnitSystem(trackable.getUnitSystem());
        ret.setUnitSystem(unit);

        //Dimension (scale)
        Vector3d dimension = trackable.getScale();
        std::vector<double> vector(3);
        for(int i = 0; i < 3; i++){
            vector[i] = dimension[0,i];
        }
        ret.setTrackableDimension(vector);

        //keyvalue taglist (multimap to map<string,vector<string>>)
        std::map<std::string, std::vector<std::string>> tagList;
        auto storageMap = trackable.getTags();
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
        ret.setKeyvalueTagList(tagList);

        return ret;
    }

    std::vector<std::byte> TrackablesSolARImpl::toBytes(std::string const& s)
    {
        std::vector<std::byte> bytes;
        bytes.reserve(std::size(s));
        std::transform(std::begin(s), std::end(s), std::back_inserter(bytes), [](char c){
            return std::byte(c);
        });

        return bytes;
    }

}
}
}
}

