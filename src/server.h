/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _GNMI_SERVER_H
#define _GNMI_SERVER_H

#include "../proto/gnmi.grpc.pb.h"

#include <sysrepo-cpp/Sysrepo.hpp>
#include <sysrepo-cpp/Connection.hpp>
#include <sysrepo-cpp/Session.hpp>

#include "encode/encode.h"

#define UNUSED(x) (void)x

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
        sr_sess = make_shared<Session>(sr_con);
        encodef = make_shared<EncodeFactory>(sr_sess);
      } catch (sysrepo::sysrepo_exception &exc) {
        cerr << "Error: Connection to sysrepo failed" << exc.what() << endl;
        exit(1);
      }
    }
    ~GNMIServer() {std::cout << "Quitting GNMI Server" << std::endl; }

    Status Capabilities(ServerContext* context,
        const CapabilityRequest* request, CapabilityResponse* response);

    Status Get(ServerContext* context,
        const GetRequest* request, GetResponse* response);

    Status Set(ServerContext* context,
        const SetRequest* request, SetResponse* response);

    Status Subscribe(ServerContext* context,
        ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private: /* Common Utils */
    /* Conversion methods between xpaths and gNMI paths */
    string gnmi_to_xpath(const Path& path);
    /* Get current time since epoch in nanosec */
    uint64_t get_time_nanosec();

  private: /* Set helpers */
    StatusCode handleUpdate(Update in, UpdateResult *out, string prefix);

  private: /* Get helpers */
    void BuildGetNotification(Notification *notification, const Path *prefix,
                              Path &path, gnmi::Encoding encoding);

  private: /* Subscribe helper */
    void BuildNotification(const SubscriptionList& request,
                           SubscribeResponse& response);
    Status handleStream( ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);
    Status handleOnce(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);
    Status handlePoll(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    sysrepo::S_Connection sr_con; //sysrepo connection
    sysrepo::S_Session sr_sess; //sysrepo session
    std::shared_ptr<EncodeFactory> encodef; //support for json ietf encoding
};

#endif //_GNMI_SERVER_H
