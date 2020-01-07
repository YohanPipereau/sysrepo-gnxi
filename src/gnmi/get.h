/*
 * Copyright 2020 Yohan Pipereau
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
