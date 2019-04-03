/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "server.h"

Status GNMIServer::Set(ServerContext *context, const SetRequest* request,
                       SetResponse* response)
{
  std::string prefix = "";

  if (request->extension_size() > 0) {
    cerr << "Extensions not implemented" << endl;
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("Extensions not implemented"));
  }

  /* Prefix for gNMI path */
  if (request->has_prefix()) {
    prefix = gnmi_to_xpath(request->prefix());
    //response->set_allocated_prefix(request->release_prefix());
  }

  /* gNMI paths to delete */
  if (request->delete__size() > 0) {

  }

  /* gNMI paths to replace */
  if (request->replace_size() > 0) {

  }

  if (request->update_size() > 0) {

  }

  response->set_timestamp(1); //TODO

  return Status::OK;
}
