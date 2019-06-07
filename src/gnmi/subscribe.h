/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _GNMI_SUBSCRIBE_H
#define _GNMI_SUBSCRIBE_H

#include <proto/gnmi.grpc.pb.h>

#include <sysrepo-cpp/Session.hpp>
#include "encode/encode.h"

using namespace gnmi;
using google::protobuf::RepeatedPtrField;
using grpc::ServerReaderWriter;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

namespace impl {

class Subscribe {
  public:
    Subscribe(sysrepo::S_Session sess, std::shared_ptr<Encode> encode)
      : sr_sess(sess), encodef(encode) {}
    ~Subscribe() {}

    Status run(ServerContext* context,
               ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    Status BuildSubsUpdate(RepeatedPtrField<Update>* updateList,
                           const Path &path, string fullpath,
                           gnmi::Encoding encoding);
    Status BuildSubscribeNotification(Notification *notification,
                                      const SubscriptionList& request);
    Status handleStream(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);
    Status handleOnce(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);
    Status handlePoll(ServerContext* context, SubscribeRequest request,
              ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    sysrepo::S_Session sr_sess; //sysrepo session
    std::shared_ptr<Encode> encodef; //support for json ietf encoding
};

}

#endif //_GNMI_SUBSCRIBE_H
