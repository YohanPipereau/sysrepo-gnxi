/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <memory>
#include <thread>
#include <chrono>
#include <string>

#include <grpc/grpc.h>

#include "subscribe.h"
#include <utils/utils.h>
#include <utils/log.h>

using namespace std;
using namespace chrono;
using google::protobuf::RepeatedPtrField;
using sysrepo::sysrepo_exception;

namespace impl {

Status
Subscribe::BuildSubsUpdate(RepeatedPtrField<Update>* updateList,
                            const Path &path, string fullpath,
                            gnmi::Encoding encoding)
{
  Update *update;
  TypedValue *gnmival;
  vector<JsonData> json_vec;
  string *json_ietf;
  int idx;
  google::protobuf::Map<string, string> *key;

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
      /* Get sysrepo subtree data corresponding to XPATH */
      try {
        json_vec = encodef->json_read(fullpath);
      } catch (invalid_argument &exc) {
        return Status(StatusCode::NOT_FOUND, exc.what());
      } catch (sysrepo_exception &exc) {
        BOOST_LOG_TRIVIAL(error) << "Fail getting items from sysrepo: "
                                 << exc.what();
        return Status(StatusCode::INVALID_ARGUMENT, exc.what());
      }

      /* Create new update message for every tree collected */
      for (auto it : json_vec) {
        update = updateList->Add();
        update->mutable_path()->CopyFrom(path);

        if (!it.key.first.empty()) {
          BOOST_LOG_TRIVIAL(debug) << "putting list entries key in gNMI path";
          idx = update->mutable_path()->elem_size() - 1;
          key = update->mutable_path()->mutable_elem(idx)->mutable_key();
          (*key)[it.key.first] = it.key.second;
        }

        gnmival = update->mutable_val();

        json_ietf = gnmival->mutable_json_ietf_val();
        *json_ietf = it.data;
      }

      break;

    case gnmi::PROTO:
      BOOST_LOG_TRIVIAL(error) << "Deviation from specification, Unsupported Yet";
      break;

    default:
      return Status(StatusCode::UNIMPLEMENTED, Encoding_Name(encoding));
  }

  return Status::OK;
}

/**
 * BuildSubscribeNotification - Build a Notification message.
 * Contrary to Get Notification, gnmi specification highly recommands to
 * put multiple <xpath, value> in the same Notification message.
 * @param notification the notification that is constructed by this function.
 * @param request the SubscriptionList from SubscribeRequest to answer to.
 */
Status
Subscribe::BuildSubscribeNotification(Notification *notification,
                                      const SubscriptionList& request)
{
  RepeatedPtrField<Update>* updateList = notification->mutable_update();
  Status status;

  switch (request.encoding()) {
    case gnmi::JSON:
    case gnmi::JSON_IETF:
      BOOST_LOG_TRIVIAL(debug) << "JSON IETF";
      break;

    case gnmi::PROTO:
      BOOST_LOG_TRIVIAL(error) << "PROTO encoding will soon be supported";
      return Status(StatusCode::UNIMPLEMENTED, Encoding_Name(request.encoding()));
      break;

    default:
      BOOST_LOG_TRIVIAL(warning) << "Unsupported Encoding "
                                 << Encoding_Name(request.encoding());
      return Status(StatusCode::UNIMPLEMENTED, Encoding_Name(request.encoding()));
  }

  // Defined refer to a long Path by a shorter one: alias
  if (request.use_aliases()) {
    BOOST_LOG_TRIVIAL(warning) << "Unsupported usage of aliases";
    return Status(StatusCode::UNIMPLEMENTED, "alias not supported");
  }

  /* Check if only updates should be sent */
  if (request.updates_only())
    BOOST_LOG_TRIVIAL(warning) << "Unsupported updates_only, send all paths";

  /* Get time since epoch in milliseconds */
  notification->set_timestamp(get_time_nanosec());

  if (request.has_prefix())
    notification->mutable_prefix()->CopyFrom(request.prefix());

  /* Fill Update RepeatedPtrField in Notification message
   * Update field contains only data elements that have changed values. */
  for (int i = 0; i < request.subscription_size(); i++) {
    Subscription sub = request.subscription(i);

    // Fetch all found counters value for a requested path
    status = BuildSubsUpdate(updateList, sub.path(), gnmi_to_xpath(sub.path()),
                             request.encoding());
    if (!status.ok()) {
      BOOST_LOG_TRIVIAL(error) << "Fail building update for "
                               << gnmi_to_xpath(sub.path());
      return status;
    }
  }

  notification->set_atomic(false);

  return Status::OK;
}

/**
 * Handles SubscribeRequest messages with STREAM subscription mode by
 * periodically sending updates to the client.
 */
Status Subscribe::handleStream(
    ServerContext* context, SubscribeRequest request,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeResponse response;
  Status status;

  // Checks that sample_interval values are not higher than INT64_MAX
  // i.e. 9223372036854775807 nanoseconds
  for (int i = 0; i < request.subscribe().subscription_size(); i++) {
    Subscription sub = request.subscribe().subscription(i);
    if (sub.sample_interval() > duration<long long, std::nano>::max().count()) {
      context->TryCancel();
      return Status(StatusCode::INVALID_ARGUMENT,
                    string("sample_interval must be less than ")
                    + to_string(INT64_MAX) + " nanoseconds");
    }
  }

  // Sends a first Notification message that updates all Subcriptions
  status = BuildSubscribeNotification(response.mutable_update(),
                                      request.subscribe());
  if (!status.ok()) {
    context->TryCancel();
    return status;
  }
  stream->Write(response);
  response.Clear();

  // Sends a SYNC message that indicates that initial synchronization
  // has completed, i.e. each Subscription has been updated once
  response.set_sync_response(true);
  stream->Write(response);
  response.Clear();

  // We use a vector of pairs instead of a map as we are going to iterate more
  // than we are going to retrieve specific keys.
  vector<pair<Subscription, time_point<high_resolution_clock>>> chronomap;
  for (int i=0; i<request.subscribe().subscription_size(); i++) {
    Subscription sub = request.subscribe().subscription(i);
    switch (sub.mode()) {
      case SAMPLE:
        chronomap.emplace_back(sub, high_resolution_clock::now());
        break;
      default:
        BOOST_LOG_TRIVIAL(warning) << "Unsupported mode";
        // TODO: Handle ON_CHANGE and TARGET_DEFINED modes
        // Ref: 3.5.1.5.2
        break;
    }
  }

  /* Periodically updates paths that require SAMPLE updates
   * Note : There is only one Path per Subscription, but repeated
   * Subscriptions in a SubscriptionList, each Subscription can
   * have its own sample interval */
  while(!context->IsCancelled()) {
    auto start = high_resolution_clock::now();

    SubscribeRequest updateRequest(request);
    SubscriptionList* updateList(updateRequest.mutable_subscribe());
    updateList->clear_subscription();

    for (auto& pair : chronomap) {
      duration<long long, std::nano> duration =
        high_resolution_clock::now()-pair.second;
      if (duration > nanoseconds{pair.first.sample_interval()}) {
        pair.second = high_resolution_clock::now();
        Subscription* sub = updateList->add_subscription();
        sub->CopyFrom(pair.first);
      }
    }

    if (updateList->subscription_size() > 0) {
      status = BuildSubscribeNotification(response.mutable_update(),
                                          updateRequest.subscribe());
      if(!status.ok()) {
        context->TryCancel();
        return status;
      }
      stream->Write(response);
      response.Clear();
    }

    // Caps the loop at 5 iterations per second
    auto loopTime = high_resolution_clock::now() - start;
    this_thread::sleep_for(milliseconds(200) - loopTime);
  }

  return Status::OK;
}

/**
 * Handles SubscribeRequest messages with ONCE subscription mode by updating
 * all the Subscriptions once, sending a SYNC message, then closing the RPC.
 */
Status Subscribe::handleOnce(ServerContext* context, SubscribeRequest request,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  Status status;

  // Sends a Notification message that updates all Subcriptions once
  SubscribeResponse response;
  status = BuildSubscribeNotification(response.mutable_update(),
                                      request.subscribe());
  if (!status.ok()) {
    context->TryCancel();
    return status;
  }

  stream->Write(response);
  response.Clear();

  // Sends a message that indicates that initial synchronization
  // has completed, i.e. each Subscription has been updated once
  response.set_sync_response(true);
  stream->Write(response);
  response.Clear();

  return Status::OK;
}

/**
 * Handles SubscribeRequest messages with POLL subscription mode by updating
 * all the Subscriptions each time a Poll request is received.
 */
Status Subscribe::handlePoll(ServerContext* context, SubscribeRequest request,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeRequest subscription = request;
  Status status;

  while (stream->Read(&request)) {
    switch (request.request_case()) {
      case request.kPoll:
        {
          // Sends a Notification message that updates all Subcriptions once
          SubscribeResponse response;
          status = BuildSubscribeNotification(response.mutable_update(),
                                              subscription.subscribe());
          if (!status.ok()) {
            context->TryCancel();
            return status;
          }
          stream->Write(response);
          response.Clear();
          break;
        }
      case request.kAliases:
        return Status(StatusCode::UNIMPLEMENTED, "Aliases not implemented yet");
      case request.kSubscribe:
        return Status(StatusCode::INVALID_ARGUMENT,
                      "A SubscriptionList has already been received for this RPC");
      default:
        return Status(StatusCode::INVALID_ARGUMENT,
                      "Unknown content for SubscribeRequest message");
    }
  }

  return Status::OK;
}

/**
 * Handles the first SubscribeRequest message.
 * If it does not have the "subscribe" field set, the RPC MUST be cancelled.
 * Ref: 3.5.1.1
 */
Status Subscribe::run(ServerContext* context,
                 ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeRequest request;

  stream->Read(&request);

  if (request.extension_size() > 0) {
    BOOST_LOG_TRIVIAL(error) << "Extensions not implemented";
    return Status(StatusCode::UNIMPLEMENTED, "Extensions not implemented");
  }

  if (!request.has_subscribe()) {
    context->TryCancel();
    return Status(StatusCode::INVALID_ARGUMENT,
                  "SubscribeRequest needs non-empty SubscriptionList");
  }

  switch (request.subscribe().mode()) {
    case SubscriptionList_Mode_STREAM:
      return handleStream(context, request, stream);
    case SubscriptionList_Mode_ONCE:
      return handleOnce(context, request, stream);
    case SubscriptionList_Mode_POLL:
      return handlePoll(context, request, stream);
    default:
      BOOST_LOG_TRIVIAL(error) << "Unknown subscription mode";
      return Status(StatusCode::UNKNOWN, "Unknown subscription mode");
  }

  return Status::OK;
}

}
