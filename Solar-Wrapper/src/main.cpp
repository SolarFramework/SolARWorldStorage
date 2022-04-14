/**
* World Storage API
* API ensuring interoperability between an authoring tool and a World Storage service
*
* The version of the OpenAPI document: 0.0.1
*
*
* NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
* https://openapi-generator.tech
* Do not edit the class manually.
*/


#include "pistache/endpoint.h"
#include "pistache/http.h"
#include "pistache/router.h"
#include <boost/log/core.hpp>
#ifdef __linux__
#include <vector>
#include <signal.h>
#include <unistd.h>
#endif

#include "core/Log.h"


#include "xpcf/xpcf.h"
#include "TrackablesSolARImpl.h"
#include "WorldAnchorsSolARImpl.h"
#include "WorldLinksSolARImpl.h"
#include "DefaultSolARImpl.h"

#define PISTACHE_SERVER_THREADS     2
#define PISTACHE_SERVER_MAX_REQUEST_SIZE 32768
#define PISTACHE_SERVER_MAX_RESPONSE_SIZE 32768


namespace xpcf = org::bcom::xpcf;

static Pistache::Http::Endpoint *httpEndpoint;
#ifdef __linux__
static void sigHandler [[noreturn]] (int sig){
    switch(sig){
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGHUP:
        default:
            httpEndpoint->shutdown();
            break;
    }
    exit(0);
}

static void setUpUnixSignals(std::vector<int> quitSignals) {
    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);
    for (auto sig : quitSignals)
        sigaddset(&blocking_mask, sig);

    struct sigaction sa;
    sa.sa_handler = sigHandler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}
#endif

using namespace org::openapitools::server::implem;
using namespace SolAR;

int main() {

    #ifdef __linux__
        std::vector<int> sigs{SIGQUIT, SIGINT, SIGTERM, SIGHUP};
        setUpUnixSignals(sigs);
    #endif

    try {

        #if NDEBUG
            boost::log::core::get()->set_logging_enabled(false);
        #endif

        //init the logger
        LOG_ADD_LOG_TO_CONSOLE();
        LOG_INFO("program is running");

        /* instantiate component manager*/
        /* this is needed in dynamic mode */
        SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();

        if(xpcfComponentManager->load("SolARSample_World_Storage_conf.xml")!=org::bcom::xpcf::_SUCCESS)
        {
            LOG_ERROR("Failed to load the configuration file SolARSample_World_Storage_conf.xml");
            return -1;
        }
        auto worldStorage = xpcfComponentManager->resolve<SolAR::api::storage::IWorldGraphManager>();

        Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));

        httpEndpoint = new Pistache::Http::Endpoint((addr));
        auto router = std::make_shared<Pistache::Rest::Router>();

        auto opts = Pistache::Http::Endpoint::options()
            .threads(PISTACHE_SERVER_THREADS);
        opts.flags(Pistache::Tcp::Options::ReuseAddr);
        opts.maxRequestSize(PISTACHE_SERVER_MAX_REQUEST_SIZE);
        opts.maxResponseSize(PISTACHE_SERVER_MAX_RESPONSE_SIZE);
        httpEndpoint->init(opts);

        TrackablesSolARImpl TrackablesApiserver(router, worldStorage);
        TrackablesApiserver.init();

        WorldAnchorsSolARImpl WorldAnchorsSolARImpl(router, worldStorage);
        WorldAnchorsSolARImpl.init();

        WorldLinksSolARImpl WorldLinksSolARImpl(router, worldStorage);
        WorldLinksSolARImpl.init();

        DefaultSolARImpl DefaultSolARImpl(router);
        DefaultSolARImpl.init();

        httpEndpoint->setHandler(router->handler());
        httpEndpoint->serve();

        httpEndpoint->shutdown();

    }

    catch (xpcf::Exception e)
    {
        LOG_ERROR("Exception raised : \n     {}", e.what())
        return -1;
    }
    return 0;

}
