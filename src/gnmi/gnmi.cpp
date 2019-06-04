/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "gnmi.h"

#include "get.h"
#include "set.h"
#include "subscribe.h"

#include <utils/log.h>

using sysrepo::DS_RUNNING;
using sysrepo::Session;

/*
 * Implementing Authorization requires knowing the identity of the peer.
 * For Mutual TLS authentication: it is client x509 cert common name
 * For UserPass authentication: it is the username
 */
sysrepo::S_Session GNMIService::createSession(ServerContext *context)
{
  auto idv = context->auth_context()->GetPeerIdentity();

  if (idv.empty()) {
    BOOST_LOG_TRIVIAL(warning) << "NO ACCESS CONTROL";
    return sr_sess;
  }

  grpc::string_ref id = idv.front();

  BOOST_LOG_TRIVIAL(debug) << "[DEBUG] Identity is: " << id;

  return make_shared<Session>(sr_con, static_cast<sr_datastore_t>(DS_RUNNING),
                                       SR_SESS_ENABLE_NACM, id.data());
}

Status GNMIService::Set(ServerContext *context, const SetRequest* request,
                       SetResponse* response)
{
  sysrepo::S_Session sess = createSession(context);

  impl::Set rpc(sess, encodef);

  return rpc.run(request, response);
}

Status GNMIService::Get(ServerContext *context, const GetRequest* request,
                        GetResponse* response)
{
  sysrepo::S_Session sess = createSession(context);

  impl::Get rpc(sess, encodef);

  return rpc.run(request, response);
}

Status GNMIService::Subscribe(ServerContext* context,
                 ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  sysrepo::S_Session sess = createSession(context);

  impl::Subscribe rpc(sess, encodef);

  return rpc.run(context, stream);

  return Status::OK;
}

