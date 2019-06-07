/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _GNMI_SET_H
#define _GNMI_SET_H

#include <proto/gnmi.grpc.pb.h>

#include <sysrepo-cpp/Session.hpp>
#include "encode/encode.h"

using namespace gnmi;
using grpc::Status;
using grpc::StatusCode;

namespace impl {

class Set {
  public:
    Set(sysrepo::S_Session sess, std::shared_ptr<Encode> encode)
      : sr_sess(sess), encodef(encode) {}
    ~Set() {}

    Status run(const SetRequest* request, SetResponse* response);

  private:
    StatusCode handleUpdate(Update in, UpdateResult *out, string prefix);

  private:
    sysrepo::S_Session sr_sess; //sysrepo session
    shared_ptr<Encode> encodef; //support for json ietf encoding
};

}

#endif //_GNMI_SET_H
