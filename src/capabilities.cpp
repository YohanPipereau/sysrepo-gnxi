/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpc/grpc.h>

#include "server.h"

using namespace grpc;
using namespace gnmi;

Status GNMIServer::Capabilities(ServerContext *context,
                                 const CapabilityRequest* request,
                                 CapabilityResponse* response)
{
  return Status(StatusCode::UNIMPLEMENTED,
                grpc::string("'Capabilities' method not implemented yet"));
}
