/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "../proto/gnmi.grpc.pb.h"

#include <sysrepo-cpp/Sysrepo.hpp>
#include <sysrepo-cpp/Connection.hpp>
#include <sysrepo-cpp/Session.hpp>

using namespace grpc;
using namespace gnmi;
using sysrepo::Session;
using sysrepo::Connection;

class GNMIServer final : public gNMI::Service
{
  public:
    GNMIServer() : sr_sess(std::make_shared<Connection>(Connection("app_name"))) {}

    Status Capabilities(ServerContext* context,
        const CapabilityRequest* request, CapabilityResponse* response);

    Status Get(ServerContext* context,
        const GetRequest* request, GetResponse* response);

    Status Set(ServerContext* context,
        const SetRequest* request, SetResponse* response);

    Status Subscribe(ServerContext* context,
        ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    Session sr_sess; //sysrepo session
};
