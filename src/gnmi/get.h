/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _GNMI_GET_H
#define _GNMI_GET_H

#include <proto/gnmi.grpc.pb.h>

#include <sysrepo-cpp/Session.hpp>
#include "encode/encode.h"

using namespace gnmi;
using grpc::Status;
using grpc::StatusCode;
using google::protobuf::RepeatedPtrField;

namespace impl {

class Get {
  public:
    Get(sysrepo::S_Session sess, std::shared_ptr<Encode> encode)
      : sr_sess(sess), encodef(encode) {}
    ~Get() {}

    Status run(const GetRequest* req, GetResponse* response);

  private:
    Status BuildGetNotification(Notification *notification, const Path *prefix,
                                const Path &path, gnmi::Encoding encoding);
    Status BuildGetUpdate(RepeatedPtrField<Update>* updateList,
                          const Path &path, string fullpath,
                          gnmi::Encoding encoding);

  private:
    sysrepo::S_Session sr_sess; //sysrepo session
    shared_ptr<Encode> encodef; //support for json ietf encoding
};

}

#endif //_GNMI_GET_H
