/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "../proto/gnmi.grpc.pb.h"

#include <sysrepo-cpp/Sysrepo.hpp>
#include <sysrepo-cpp/Connection.hpp>
#include <sysrepo-cpp/Session.hpp>

using namespace grpc;
using namespace gnmi;
using namespace std;
using sysrepo::Session;
using sysrepo::Connection;


class GNMIServer final : public gNMI::Service
{
  public:
    GNMIServer() {
      sr_con = make_shared<Connection>("app");
      sr_sess = make_shared<Session>(sr_con);
    }

    Status Capabilities(ServerContext* context,
        const CapabilityRequest* request, CapabilityResponse* response);

    Status Get(ServerContext* context,
        const GetRequest* request, GetResponse* response);

    Status Set(ServerContext* context,
        const SetRequest* request, SetResponse* response);

    Status Subscribe(ServerContext* context,
        ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    std::shared_ptr<Connection> sr_con;
    std::shared_ptr<Session> sr_sess; //sysrepo session
};
