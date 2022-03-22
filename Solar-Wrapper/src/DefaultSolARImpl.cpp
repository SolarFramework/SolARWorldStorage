#include "DefaultSolARImpl.h"

namespace org {
namespace openapitools {
namespace server {
namespace implem {

    DefaultSolARImpl::DefaultSolARImpl(const std::shared_ptr<Pistache::Rest::Router>& rtr)
        : DefaultApi(rtr)
    {
    }

    void DefaultSolARImpl::get_version(Pistache::Http::ResponseWriter &response){
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        response.send(Pistache::Http::Code::Ok, "Version 0.0.3");
    }

    void DefaultSolARImpl::get_ping(Pistache::Http::ResponseWriter &response) {
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Text, Plain));
        response.send(Pistache::Http::Code::Ok, "Pong");
    }

}
}
}
}
