/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "server.h"

using namespace grpc;
using namespace gnmi;
using namespace std;
using sysrepo::Yang_Schemas;
using google::protobuf::FileOptions;

Status GNMIServer::Capabilities(ServerContext *context,
                                 const CapabilityRequest* request,
                                 CapabilityResponse* response)
{
  UNUSED(context);
  shared_ptr<Yang_Schemas> schemas;
  string gnmi_version;
  FileOptions fopts;

  if (request->extension_size() > 0) {
    cerr << "Extensions not implemented" << endl;
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("Extensions not implemented"));
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
    //response->add_supported_encodings(gnmi::Encoding::JSON_IETF);

  } catch (const exception &exc) {
    cerr << "ERROR" << exc.what() << endl;
    return Status(StatusCode::INTERNAL, grpc::string("Fail getting schemas"));
  }

  return Status::OK;
}
