/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "gnmi.h"

#include "get.h"
#include "set.h"
#include "subscribe.h"

Status GNMIService::Set(ServerContext *context, const SetRequest* request,
                       SetResponse* response)
{
  (void)context;
  impl::Set rpc(sr_sess, encodef);

  return rpc.run(request, response);
}

Status GNMIService::Get(ServerContext *context, const GetRequest* request,
                        GetResponse* response)
{
  (void)context;
  impl::Get rpc(sr_sess, encodef);

  return rpc.run(request, response);
}

Status GNMIService::Subscribe(ServerContext* context,
                 ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeRequest request;
  impl::Subscribe rpc(sr_sess, encodef);

  return rpc.run(context, stream);

  return Status::OK;
}

