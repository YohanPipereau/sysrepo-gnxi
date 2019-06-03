/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpc/grpc.h>

#include "gnmi.h"
#include "encode/encode.h"
#include <utils/utils.h>
#include <utils/log.h>

using namespace std;
using google::protobuf::RepeatedPtrField;
using sysrepo::sysrepo_exception;

Status
GNMIService::BuildUpdate(RepeatedPtrField<Update>* updateList, const Path &path,
                         string fullpath, gnmi::Encoding encoding)
{
  Update *update;
  TypedValue *gnmival;
  string *json_ietf;

  /* Create Update message */
  update = updateList->Add();
  update->mutable_path()->CopyFrom(path);
  gnmival = update->mutable_val();

  /* Refresh configuration data from current session */
  sr_sess->refresh();

  /* Create appropriate TypedValue message based on encoding */
  switch (encoding) {
    case gnmi::JSON:
    case gnmi::JSON_IETF:
      json_ietf = gnmival->mutable_json_ietf_val();
      /* Get sysrepo subtree data corresponding to XPATH */
      try {
        *json_ietf = encodef->getEncoding(EncodeFactory::Encoding::JSON_IETF)->read(fullpath);
      } catch (invalid_argument &exc) {
        return Status(StatusCode::NOT_FOUND, exc.what());
      } catch (sysrepo_exception &exc) {
        BOOST_LOG_TRIVIAL(error) << "Fail getting items from sysrepo "
                                 << exc.what();
        return Status(StatusCode::INVALID_ARGUMENT, exc.what());
      }
      break;

    default:
      return Status(StatusCode::UNIMPLEMENTED, Encoding_Name(encoding));
  }

  return Status::OK;
}

/*
 * Build Get Notifications - Build a Notification message.
 * Contrary to Subscribe Notifications, A new notification message must be
 * created for every path of the GetRequest.
 * There can still be multiple paths in GetResponse if requested path
 * is a directory path.
 *
 * IMPORTANT : we have choosen to have a stateless implementation of
 * gNMI so deleted path in Notification message will always be empty.
 */
Status
GNMIService::BuildGetNotification(Notification *notification, const Path *prefix,
                                 const Path &path, gnmi::Encoding encoding)
{
  /* Data elements that have changed values */
  RepeatedPtrField<Update>* updateList = notification->mutable_update();
  string fullpath = "";

  /* Get time since epoch in milliseconds */
  notification->set_timestamp(get_time_nanosec());

  /* Put Request prefix as Response prefix */
  if (prefix != nullptr) {
    string str = gnmi_to_xpath(*prefix);
    BOOST_LOG_TRIVIAL(debug) << "prefix is " << str;
    notification->mutable_prefix()->CopyFrom(*prefix);
    fullpath += str;
  }

  fullpath += gnmi_to_xpath(path);
  BOOST_LOG_TRIVIAL(debug) << "GetRequest Path " << fullpath;


  /* TODO Check DATA TYPE in {ALL,CONFIG,STATE,OPERATIONAL}
   * This is interesting for NMDA architecture
   * req->type() : GetRequest_DataType_ALL,CONFIG,STATE,OPERATIONAL
   */
  return BuildUpdate(updateList, path, fullpath, encoding);
}

/* Verify request fields are correct */
static inline Status verifyGetRequest(const GetRequest *request)
{
  switch (request->encoding()) {
    case gnmi::JSON:
    case gnmi::JSON_IETF:
      break;

    default:
      BOOST_LOG_TRIVIAL(warning) << "Unsupported Encoding "
                                 << Encoding_Name(request->encoding());
      return Status(StatusCode::UNIMPLEMENTED,
                    Encoding_Name(request->encoding()));
  }

  if (!GetRequest_DataType_IsValid(request->type())) {
    BOOST_LOG_TRIVIAL(warning) << "Invalid Data Type in Get Request "
                               << GetRequest_DataType_Name(request->type());
    return Status(StatusCode::UNIMPLEMENTED,
                  GetRequest_DataType_Name(request->type()));
  }

  if (request->use_models_size() > 0) {
    BOOST_LOG_TRIVIAL(warning) << "use_models unsupported, ALL are used";
    return Status(StatusCode::UNIMPLEMENTED, "use_model feature unsupported");
  }

  if (request->extension_size() > 0) {
    BOOST_LOG_TRIVIAL(warning) << "extension unsupported";
    return Status(StatusCode::UNIMPLEMENTED, "extension feature unsupported");
  }

  return Status::OK;
}

/* Implement gNMI Get RPC */
Status GNMIService::Get(ServerContext *context, const GetRequest* req,
                        GetResponse* response)
{
  (void)context;
  RepeatedPtrField<Notification> *notificationList;
  Notification *notification;
  Status status;

  status = verifyGetRequest(req);
  if (!status.ok())
    return status;

  BOOST_LOG_TRIVIAL(debug) << "GetRequest DataType "
                           << GetRequest::DataType_Name(req->type()) << ","
                           << "GetRequest Encoding "
                           << Encoding_Name(req->encoding());

  /* Run through all paths */
  notificationList = response->mutable_notification();
  for (auto path : req->path()) {
    notification = notificationList->Add();

    if (req->has_prefix())
      status = BuildGetNotification(notification, &req->prefix(), path, req->encoding());
    else
      status = BuildGetNotification(notification, nullptr, path, req->encoding());

    if (!status.ok()) {
      BOOST_LOG_TRIVIAL(error) << "Fail building get notification "
                               << status.error_message();
      return status;
    }
  }

  return Status::OK;
}
