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

#include "gnmi.h"
#include <utils/log.h>

using namespace gnmi;
using namespace std;
using sysrepo::Yang_Schemas;
using google::protobuf::FileOptions;

Status GNMIService::Capabilities(ServerContext *context,
                                 const CapabilityRequest* request,
                                 CapabilityResponse* response)
{
  (void)context;
  shared_ptr<Yang_Schemas> schemas;
  string gnmi_version;
  FileOptions fopts;

  if (request->extension_size() > 0) {
    BOOST_LOG_TRIVIAL(error) << "Extensions not implemented";
    return Status(StatusCode::UNIMPLEMENTED, "Extensions not implemented");
  }

  try {
    schemas = sr_sess->list_schemas();

    for (unsigned int i = 0; i < schemas->schema_cnt(); i++) {
      auto model = response->add_supported_models();
      model->set_name(schemas->schema(i)->module_name());
      model->set_version(schemas->schema(i)->revision()->revision());
    }

    gnmi_version = response->GetDescriptor()->file()->options()
                            .GetExtension(gnmi::gnmi_service);
    response->set_gnmi_version(gnmi_version);

    //Encoding used in TypedValue for responses
    //response->add_supported_encodings(gnmi::Encoding::JSON);
    //response->add_supported_encodings(gnmi::Encoding::BYTES);
    //response->add_supported_encodings(gnmi::Encoding::PROTO);
    //response->add_supported_encodings(gnmi::Encoding::ASCII);
    response->add_supported_encodings(gnmi::Encoding::JSON_IETF);

  } catch (const exception &exc) {
    BOOST_LOG_TRIVIAL(error) << exc.what();
    return Status(StatusCode::INTERNAL, "Fail getting schemas");
  }

  return Status::OK;
}
