/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "server.h"

Status GNMIServer::Get(ServerContext *context, const GetRequest* request,
                        GetResponse* response)
{
  UNUSED(context); UNUSED(request); UNUSED(response);
  return Status(StatusCode::UNIMPLEMENTED,
                grpc::string("'Get' method not implemented yet"));
}
