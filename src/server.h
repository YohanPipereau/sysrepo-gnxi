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
    GNMIServer(string app_name) {
      try {
        sr_con = make_shared<Connection>(app_name.c_str());
        sr_sess = make_shared<Session>(Session(sr_con));
      } catch (sysrepo::sysrepo_exception &exc) {
        cerr << "Error: Connection to sysrepo failed" << exc.what() << endl;
        exit(1);
      }
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
    /* Conversion methods between xpaths and gNMI paths */
    void xpath_to_gnmi(Path *path, string xpath);
    string gnmi_to_xpath(const Path& path);

    /* Get current time since epoch in nanosec */
    uint64_t get_time_nanosec();

    /* Set helper */
    StatusCode handleUpdate(Update in, UpdateResult *out, string prefix);

    /* Subscribe helper */
    void BuildNotification(const SubscriptionList & request,
                           SubscribeResponse& response);

    Status handleStream( ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

    Status handleOnce(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

    Status handlePoll(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    sysrepo::S_Connection sr_con;
    sysrepo::S_Session sr_sess; //sysrepo session
};
