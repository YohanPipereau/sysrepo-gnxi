/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpc/grpc.h>
#include <sysrepo-cpp/Struct.hpp>

#include "server.h"

using namespace std;
using google::protobuf::RepeatedPtrField;
using sysrepo::S_Val;
using sysrepo::S_Iter_Value;
using sysrepo::sysrepo_exception;

/*
 * Build Get Notifications
 * Contrary to Subscribe Notifications, A new notification message must be
 * created for every path of the GetRequest.
 * There can still be multiple paths in GetResponse if requested path
 * is a directory path.
 *
 * IMPORTANT : we have choosen to have a stateless implementation of
 * gNMI so deleted path in Notification message will always be empty.
 */
void
GNMIServer::BuildGetNotification(Notification *notification, const Path *prefix,
                                 Path &path, gnmi::Encoding encoding)
{
  /* Data elements that have changed values */
  RepeatedPtrField<Update>* updateList = notification->mutable_update();
  Update *update;
  TypedValue *gnmival;
  string fullpath = "";
  S_Iter_Value iter;
  S_Val val;

  /* Get time since epoch in milliseconds */
  notification->set_timestamp(get_time_nanosec());

  /* Put Request prefix as Response prefix */
  if (prefix != nullptr) {
    string str = gnmi_to_xpath(*prefix);
    cerr << "DEBUG: prefix is" << str << endl;
    notification->mutable_prefix()->CopyFrom(*prefix);
    fullpath += str;
  }

  fullpath += gnmi_to_xpath(path);

  cout << "DEBUG: Building Notification for " << fullpath << endl;

  update = updateList->Add();
  update->mutable_path()->CopyFrom(path);
  gnmival = update->mutable_val();
  switch (encoding) {
    case JSON:
      gnmival->mutable_json_ietf_val(); //TODO return a string*
      break;
    default:
      cerr << "ERROR: Unsupported encoding" << endl;
      return;
  }

  /* Get sysrepo subtree data corresponding to XPATH */
  try {
    iter = sr_sess->get_items_iter(fullpath.c_str());
    if (iter == nullptr) { //nothing was found for this xpath
      cerr << "ERROR: No data in sysrepo for " << fullpath << endl;
      //TODO throw exception or return error code
      return;
    }

    while (sr_sess->get_item_next(iter) != nullptr) {
      cout << "DEBUG: " << endl;
    }
  } catch (sysrepo_exception &exc) {
    cerr << "ERROR: Fail getting items from sysrepo "
         << "l." << __LINE__ << " " << exc.what()
         << endl;
    return;
  }

  cout << "DEBUG: End of Notification" << endl;

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

  cout << "DEBUG: Get RPC"
       << "\n\tDataType: " << GetRequest::DataType_Name(request->type())
       << "\n\tEncoding: " << Encoding_Name(request->encoding())
       << endl;

  /* handle Paths */
  RepeatedPtrField<Notification> *notificationList = response->mutable_notification();
  Notification *notification;

  for (auto path : request->path()) {
    cout << "\tPath: "
         << gnmi_to_xpath(request->prefix()) + gnmi_to_xpath(path)
         << endl;
    notification = notificationList->Add();

    if (request->has_prefix())
      BuildGetNotification(notification, &request->prefix(), path,
                           request->encoding());
    else
      BuildGetNotification(notification, nullptr, path,
                           request->encoding());

  }

  return Status::OK;
}
