/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "server.h"
#include "encode/json_ietf.h"

using namespace sysrepo;

StatusCode GNMIServer::handleUpdate(Update in, UpdateResult *out, string prefix)
{
  shared_ptr<Val> sval;
  //Parse request
  if (!in.has_path() || !in.has_val()) {
    cerr << "ERROR: Update no path or value" << endl;
    return StatusCode::INVALID_ARGUMENT;
  }

  string fullpath = prefix + gnmi_to_xpath(in.path());
  TypedValue reqval = in.val();
  cerr << "DEBUG: Update" << fullpath << endl;

  switch (reqval.value_case()) {
    case gnmi::TypedValue::ValueCase::kStringVal:
      sval = make_shared<Val>(reqval.string_val().c_str());
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kIntVal:
      sval = make_shared<Val>(reqval.int_val(), SR_INT64_T);
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kUintVal:
      sval = make_shared<Val>(reqval.uint_val(), SR_UINT64_T);
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kBoolVal:
      sval = make_shared<Val>(reqval.bool_val());
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kBytesVal:
      throw std::invalid_argument("Unsupported BYTES Encoding");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::kFloatVal:
      sval = make_shared<Val>(static_cast<double>(reqval.float_val()));
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kDecimalVal:
      throw std::invalid_argument("Unsupported Decimal64 type");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::kLeaflistVal:
      throw std::invalid_argument("Unsupported leaflist type");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::kAnyVal:
      throw std::invalid_argument("Unsupported PROTOBUF Encoding");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::kJsonVal:
      throw std::invalid_argument("Unsupported JSON Encoding");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::kJsonIetfVal:
      throw std::invalid_argument("Unsupported JSON Encoding");
      break;
    case gnmi::TypedValue::ValueCase::kAsciiVal:
      throw std::invalid_argument("Unsupported ASCII Encoding");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::kProtoBytes:
      throw std::invalid_argument("Unsupported PROTOBUF BYTE Encoding");
      return StatusCode::UNIMPLEMENTED;
    case gnmi::TypedValue::ValueCase::VALUE_NOT_SET:
      throw std::invalid_argument("Value not set");
      return StatusCode::INVALID_ARGUMENT;
    default:
      throw std::invalid_argument("Unknown value type");
      return StatusCode::INVALID_ARGUMENT;
  }


  //Fill in Reponse
  out->set_allocated_path(in.release_path());

  return StatusCode::OK;
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
    response->mutable_prefix()->CopyFrom(request->prefix());
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
        cerr << __FUNCTION__ << __LINE__ << "ERROR" << exc.what() << endl;
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
      } catch (const invalid_argument &exc) {
        cerr << __FUNCTION__ << __LINE__ << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INVALID_ARGUMENT, grpc::string(exc.what()));
      } catch (const sysrepo_exception &exc) {
        cerr << __FUNCTION__ << __LINE__ << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INTERNAL, grpc::string(exc.what()));
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
      } catch (const invalid_argument &exc) {
        cerr << __FUNCTION__ << __LINE__ << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INVALID_ARGUMENT, grpc::string(exc.what()));
      } catch (const sysrepo_exception &exc) {
        cerr << __FUNCTION__ << __LINE__ << "ERROR" << exc.what() << endl;
        return Status(StatusCode::INTERNAL, grpc::string(exc.what()));
      }
      res->set_op(gnmi::UpdateResult::UPDATE);
    }
  }

  try {
    sr_sess->commit();
  } catch (const exception &exc) {
    cerr << __FUNCTION__ << __LINE__ << "ERROR" << exc.what() << endl;
    return Status(StatusCode::INTERNAL, grpc::string("commit failed"));
  }

  return Status::OK;
}
