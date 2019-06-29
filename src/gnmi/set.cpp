/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "set.h"
#include "encode/encode.h"
#include <utils/utils.h>
#include <utils/log.h>

using namespace sysrepo;
using namespace std;

namespace impl {

StatusCode Set::handleUpdate(Update in, UpdateResult *out, string prefix)
{
  shared_ptr<Val> sval;
  //Parse request
  if (!in.has_path() || !in.has_val()) {
    BOOST_LOG_TRIVIAL(error) << "Update no path or value";
    return StatusCode::INVALID_ARGUMENT;
  }

  string fullpath = prefix + gnmi_to_xpath(in.path());
  TypedValue reqval = in.val();
  BOOST_LOG_TRIVIAL(debug) << "Update" << fullpath;

  switch (reqval.value_case()) {
    case gnmi::TypedValue::ValueCase::kStringVal: /* No encoding */
      sval = make_shared<Val>(reqval.string_val().c_str());
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kIntVal: /* No Encoding */
      sval = make_shared<Val>(reqval.int_val(), SR_INT64_T);
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kUintVal: /* No Encoding */
      sval = make_shared<Val>(reqval.uint_val(), SR_UINT64_T);
      sr_sess->set_item(fullpath.c_str(), sval);
      break;
    case gnmi::TypedValue::ValueCase::kBoolVal: /* No Encoding */
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
    case gnmi::TypedValue::ValueCase::kDecimalVal: /* No Encoding */
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
      try {
        encodef->json_update(reqval.json_ietf_val());
      } catch (runtime_error &err) {
        //wrong input field must reply an error to gnmi client
        throw std::invalid_argument(err.what());
        return StatusCode::INVALID_ARGUMENT;
      } catch (invalid_argument &err) {
        throw std::invalid_argument(err.what());
        return StatusCode::INVALID_ARGUMENT;
      }
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

Status Set::run(const SetRequest* request, SetResponse* response)
{
  std::string prefix = "";

  if (request->extension_size() > 0) {
    BOOST_LOG_TRIVIAL(error) << "Extensions not implemented";
    return Status(StatusCode::UNIMPLEMENTED, "Extensions not implemented");
  }

  response->set_timestamp(get_time_nanosec());

  /* Prefix for gNMI path */
  if (request->has_prefix()) {
    prefix = gnmi_to_xpath(request->prefix());
    BOOST_LOG_TRIVIAL(debug) << "prefix is " << prefix;
    response->mutable_prefix()->CopyFrom(request->prefix());
  }

  /* gNMI paths to delete */
  if (request->delete__size() > 0) {
    for (auto delpath : request->delete_()) {
      //Parse request and config sysrepo
      string fullpath = prefix + gnmi_to_xpath(delpath);
      BOOST_LOG_TRIVIAL(debug) << "Delete " << fullpath;
      try {
        sr_sess->delete_item(fullpath.c_str()); //EDIT_DEFAULT option
      } catch (const exception &exc) {
        BOOST_LOG_TRIVIAL(error) << exc.what();
        return Status(StatusCode::INTERNAL, "delete item failed");
      }
      //Fill in Reponse
      UpdateResult* res = response->add_response();
      *(res->mutable_path()) = delpath;
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
        BOOST_LOG_TRIVIAL(error) << exc.what();
        return Status(StatusCode::INVALID_ARGUMENT, exc.what());
      } catch (const sysrepo_exception &exc) {
        BOOST_LOG_TRIVIAL(error) << exc.what();
        return Status(StatusCode::INTERNAL, exc.what());
      } catch (const exception &exc) { //Any other exception
        BOOST_LOG_TRIVIAL(error) << exc.what();
        return Status(StatusCode::INTERNAL, exc.what());
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
        BOOST_LOG_TRIVIAL(error) << exc.what();
        return Status(StatusCode::INVALID_ARGUMENT, exc.what());
      } catch (const sysrepo_exception &exc) {
        BOOST_LOG_TRIVIAL(error) << exc.what();
        return Status(StatusCode::INTERNAL, exc.what());
      }
      res->set_op(gnmi::UpdateResult::UPDATE);
    }
  }

  try {
    sr_sess->commit();
  } catch (const exception &exc) {
    BOOST_LOG_TRIVIAL(error) << exc.what();
    return Status(StatusCode::INTERNAL, "commit failed");
  }

  return Status::OK;
}

}
