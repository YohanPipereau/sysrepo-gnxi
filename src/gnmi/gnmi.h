/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _GNMI_SERVER_H
#define _GNMI_SERVER_H

#include <proto/gnmi.grpc.pb.h>

#include <sysrepo-cpp/Sysrepo.hpp>
#include <sysrepo-cpp/Connection.hpp>
#include <sysrepo-cpp/Session.hpp>

#include "encode/encode.h"

using namespace grpc;
using namespace gnmi;

using sysrepo::Session;
using sysrepo::Connection;
using google::protobuf::RepeatedPtrField;
using std::make_shared;

class GNMIService final : public gNMI::Service
{
  public:
    GNMIService(string app) {
      try {
        sr_con = make_shared<Connection>(app.c_str(), SR_CONN_DAEMON_REQUIRED);
        sr_sess = make_shared<Session>(sr_con);
        encodef = make_shared<Encode>(sr_sess);
      } catch (sysrepo::sysrepo_exception &exc) {
        std::cerr << "Connection to sysrepo failed " << exc.what() << std::endl;
        exit(1);
      }
    }
    ~GNMIService() {std::cout << "Quitting GNMI Server" << std::endl; }

    Status Capabilities(ServerContext* context,
        const CapabilityRequest* request, CapabilityResponse* response);

    Status Get(ServerContext* context,
        const GetRequest* request, GetResponse* response);

    Status Set(ServerContext* context,
        const SetRequest* request, SetResponse* response);

    Status Subscribe(ServerContext* context,
        ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    sysrepo::S_Connection sr_con; //sysrepo connection
    sysrepo::S_Session sr_sess; //sysrepo session
    shared_ptr<Encode> encodef; //support for json ietf encoding
};

#endif //_GNMI_SERVER_H
