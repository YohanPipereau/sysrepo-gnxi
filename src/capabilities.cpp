/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "server.h"

using namespace grpc;
using namespace gnmi;
using namespace std;
using sysrepo::Yang_Schemas;

Status GNMIServer::Capabilities(ServerContext *context,
                                 const CapabilityRequest* request,
                                 CapabilityResponse* response)
{
  shared_ptr<Yang_Schemas> schemas;
  cout << "In Capabilities" << endl;

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
    }

    response->set_gnmi_version("1");
    response->add_supported_encodings(gnmi::Encoding::ASCII);
  } catch (const exception &exc) {
    cerr << exc.what() << endl;
    return Status(StatusCode::INTERNAL, grpc::string("Fail getting schemas"));
  }

  return Status::OK;
}
