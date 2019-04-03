/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "server.h"

using namespace sysrepo;

int GNMIServer::handleUpdate(Update in, UpdateResult *out, string prefix)
{
  shared_ptr<Val> sval;
  //Parse request
  if (!in.has_path() || !in.has_val()) {
    cerr << "ERROR: Update no path or value" << endl;
    return -1;
  }

  string fullpath = prefix + gnmi_to_xpath(in.path());
  TypedValue reqval = in.val();
  cerr << "DEBUG: Update" << fullpath << endl;

  switch (reqval.value_case()) {
    case gnmi::TypedValue::ValueCase::kStringVal:
      sval = make_shared<Val>(reqval.string_val().c_str());
      break;
    case gnmi::TypedValue::ValueCase::kIntVal:
      sval = make_shared<Val>(reqval.int_val(), SR_INT64_T);
      break;
    case gnmi::TypedValue::ValueCase::kUintVal:
      sval = make_shared<Val>(reqval.uint_val(), SR_UINT64_T);
      break;
    case gnmi::TypedValue::ValueCase::kBoolVal:
      sval = make_shared<Val>(reqval.bool_val());
      break;
    case gnmi::TypedValue::ValueCase::kBytesVal:
      sval = make_shared<Val>(reqval.bytes_val().c_str(), SR_BINARY_T);
      break;
    case gnmi::TypedValue::ValueCase::kFloatVal:
      sval = make_shared<Val>(static_cast<double>(reqval.float_val()));
      break;
    case gnmi::TypedValue::ValueCase::kDecimalVal:
      cerr << "ERROR: Unsupported Decimal64 type" << endl;
      break;
    case gnmi::TypedValue::ValueCase::kLeaflistVal:
      cerr << "ERROR: Unsupported leaflist type" << endl;
      return -1;
    case gnmi::TypedValue::ValueCase::kAnyVal:
      cerr << "ERROR: Unsupported PROTOBUF Encoding" << endl;
      return -1;
    case gnmi::TypedValue::ValueCase::kJsonVal:
      cerr << "ERROR: Unsupported JSON Encoding" << endl;
      return -1;
    case gnmi::TypedValue::ValueCase::kJsonIetfVal:
      cerr << "ERROR: Unsupported IETF JSON Encoding" << endl;
      return -1;
    case gnmi::TypedValue::ValueCase::kAsciiVal:
      cerr << "ERROR: Unsupported ASCII Encoding" << endl;
      return -1;
    case gnmi::TypedValue::ValueCase::kProtoBytes:
      cerr << "ERROR: Unsupported PROTOBUF BYTE Encoding" << endl;
      return -1;
    case gnmi::TypedValue::ValueCase::VALUE_NOT_SET:
      cerr << "ERROR: Value not set" << endl;
      return -1;
    default:
      cerr << "ERROR: Unknown value type" << endl;
      return -1;
  }

  sr_sess->set_item(fullpath.c_str(), sval);

  //Fill in Reponse
  out->set_allocated_path(in.release_path());

  return 0;
}

Status GNMIServer::Set(ServerContext *context, const SetRequest* request,
                       SetResponse* response)
{
  std::string prefix = "";
  (void)context;

  if (request->extension_size() > 0) {
    cerr << "Extensions not implemented" << endl;
    return Status(StatusCode::UNIMPLEMENTED,
                  grpc::string("Extensions not implemented"));
  }

  response->set_timestamp(get_time_nanosec());

  /* Prefix for gNMI path */
  if (request->has_prefix()) {
    prefix = gnmi_to_xpath(request->prefix());
    cerr << "DEBUG: prefix is" << prefix << endl;
    xpath_to_gnmi(response->mutable_prefix(), prefix);
  }

  /* gNMI paths to delete */
  if (request->delete__size() > 0) {
    for (auto delpath : request->delete_()) {
      //Parse request and config sysrepo
      string fullpath = prefix + gnmi_to_xpath(delpath);
      cerr << "DEBUG: Delete" << fullpath << endl;
      try {
        sr_sess->delete_item(fullpath.c_str()); //EDIT_DEFAULT option
      } catch (const exception &exc) {
        cerr << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INTERNAL, grpc::string("delete item failed"));
      }
      //Fill in Reponse
      UpdateResult* res = response->add_response();
      res->set_allocated_path(&delpath);
      res->set_op(gnmi::UpdateResult::DELETE);
    }
  }

  /* gNMI paths with value to replace */
  if (request->replace_size() > 0) {
    for (auto &upd : request->replace()) {
      UpdateResult* res = response->add_response();
      try {
        handleUpdate(upd, res, prefix);
      } catch (const exception &exc) {
        cerr << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INTERNAL, grpc::string("set item failed"));
      }
      res->set_op(gnmi::UpdateResult::REPLACE);
    }
  }

  /* gNMI paths with value to update */
  if (request->update_size() > 0) {
    for (auto &upd : request->update()) {
      UpdateResult* res = response->add_response();
      try {
        handleUpdate(upd, res, prefix);
      } catch (const exception &exc) {
        cerr << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INTERNAL, grpc::string("set item failed"));
      }
      res->set_op(gnmi::UpdateResult::UPDATE);
    }
  }

  try {
    sr_sess->commit();
  } catch (const exception &exc) {
    cerr << "ERROR" << exc.what() << endl;
    return Status(StatusCode::INTERNAL, grpc::string("commit failed"));
  }

  return Status::OK;
}
