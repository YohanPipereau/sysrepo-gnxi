/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpc/grpc.h>

#include "server.h"

using namespace std;
using google::protobuf::RepeatedPtrField;

/*
 * Build Get Notifications
 * Contrary to Subscribe Notifications, A new notification message must be
 * created for every path of the GetRequest
 */
void
GNMIServer::BuildGetNotification(Notification *notification, const Path *prefix,
                                 Path &path)
{
  RepeatedPtrField<Update>* updateList = notification->mutable_update();

  /* Get time since epoch in milliseconds */
  notification->set_timestamp(get_time_nanosec());

  /* Put Request prefix as Response prefix */
  if (prefix != nullptr) {
    string str = gnmi_to_xpath(*prefix);
    cerr << "DEBUG: prefix is" << str << endl;
    notification->mutable_prefix()->CopyFrom(*prefix);
  }

}

/* Implement gNMI Get RPC */
Status GNMIServer::Get(ServerContext *context, const GetRequest* request,
                        GetResponse* response)
{
  /* Check DATA TYPE in {ALL,CONFIG,STATE,OPERATIONAL} */
  switch (request->type()) {
    case GetRequest_DataType_ALL:
      break;

    case GetRequest_DataType_CONFIG:
      break;

    case GetRequest_DataType_STATE:
      break;

    case GetRequest_DataType_OPERATIONAL:
      break;

    default:
      cerr << "WARN: invalid Data Type in Get Request" << endl;
      context->TryCancel();
      return Status(StatusCode::UNIMPLEMENTED,
                    grpc::string(GetRequest_DataType_Name(request->type())));
  }

  /* Check Encoding is available */
  switch (request->encoding()) {
    case JSON:
      break;

    default:
      cerr << "WARN: Unsupported Encoding" << endl;
      context->TryCancel();
      return Status(StatusCode::UNIMPLEMENTED,
                    grpc::string(Encoding_Name(request->encoding())));
  }

  if (request->use_models_size() > 0) {
    cerr << "WARN: Use models feature unsupported, ALL are used" << endl;
    context->TryCancel();
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("use_model feature unsupported"));
  }

  if (request->extension_size() > 0) {
    cerr << "WARN: Use models feature unsupported, ALL are used" << endl;
    context->TryCancel();
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("extension feature unsupported"));

  }

  cout << "DEBUG: Get RPC \n\t"
       << "DataType: " << GetRequest::DataType_Name(request->type()) << "\n\t"
       << "Encoding: " << Encoding_Name(request->encoding())
       << endl;

  /* handle Paths */
  RepeatedPtrField<Notification> *notificationList = response->mutable_notification();
  Notification *notification;

  for (auto path : request->path()) {
    notification = notificationList->Add();

    if (request->has_prefix())
      BuildGetNotification(notification, &request->prefix(), path);
    else
      BuildGetNotification(notification, nullptr, path);

  }

  return Status::OK;
}
