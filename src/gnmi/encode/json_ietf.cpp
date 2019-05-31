/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */


#include <sysrepo-cpp/Struct.hpp>
#include <sysrepo-cpp/Sysrepo.hpp>
#include <libyang/Tree_Schema.hpp>
#include <libyang/Tree_Data.hpp>

#include <utils/log.h>

#include "encode.h"

using namespace std;
using namespace libyang;


/*****************
 * CRUD - UPDATE *
 *****************/

/*
 * Parse a message encoded in JSON IETF and set fields in sysrepo.
 * @param data Input data encoded in JSON
 */
void JsonEncode::update(string data)
{
  S_Data_Node node;

  /* Parse input JSON, same options than netopeer2 edit-config */
  node = ctx->parse_data_mem(data.c_str(), LYD_JSON, LYD_OPT_EDIT |
                                                     LYD_OPT_STRICT);

  /* store Data Tree to sysrepo */
  storeTree(node);
}

/***************
 * CRUD - READ *
 ***************/

static Json::Value json_tree(sysrepo::S_Tree tree)
{
  sysrepo::S_Tree iter;
  Json::Value val;

  // run through all siblings
  for (iter = tree->first_child(); iter != nullptr; iter = iter->next()) {
    //create sibling with "node" as a parent
    switch (iter->type()) { //follows RFC 7951
      /* JSON Number */
      case SR_UINT8_T:
        val[iter->name()] = iter->data()->get_uint8();
        break;
      case SR_UINT16_T:
        val[iter->name()] = iter->data()->get_uint16();
        break;
      case SR_UINT32_T:
        val[iter->name()] = iter->data()->get_uint32();
        break;
      case SR_INT8_T:
        val[iter->name()] = iter->data()->get_int8();
        break;
      case SR_INT16_T:
        val[iter->name()] = iter->data()->get_int16();
        break;
      case SR_INT32_T:
        val[iter->name()] = iter->data()->get_int32();
        break;

      /* JSON string */
      case SR_STRING_T:
        val[iter->name()] = iter->data()->get_string();
        break;
      case SR_INT64_T:
        val[iter->name()] = to_string(iter->data()->get_int64());
        break;
      case SR_UINT64_T:
        val[iter->name()] = to_string(iter->data()->get_uint64());
        break;
      case SR_DECIMAL64_T:
        val[iter->name()] = to_string(iter->data()->get_decimal64());
        break;
      case SR_IDENTITYREF_T:
        val[iter->name()] = iter->data()->get_identityref();
        break;
      case SR_INSTANCEID_T:
        val[iter->name()] = iter->data()->get_identityref();
        break;
      case SR_BINARY_T:
        val[iter->name()] = iter->data()->get_binary();
        break;
      case SR_BITS_T:
        val[iter->name()] = iter->data()->get_bits();
        break;
      case SR_ENUM_T:
        val[iter->name()] = iter->data()->get_enum();
        break;
      case SR_BOOL_T:
        val[iter->name()] = iter->data()->get_bool() ? "true" : "false";
        break;

      /* JSON arrays */
      case SR_LIST_T:
        val[iter->name()].append(json_tree(iter));
        break;
      case SR_LEAF_EMPTY_T:
        val[iter->name()].append("null");
        break;

      /* nested JSON */
      case SR_CONTAINER_T:
      case SR_CONTAINER_PRESENCE_T:
        val[iter->name()] = json_tree(iter);
        break;

      /* Unsupported types */
      case SR_ANYDATA_T:
      case SR_ANYXML_T:
        throw invalid_argument("unsupported ANYDATA and ANYXML types");
        break;

      default:
        BOOST_LOG_TRIVIAL(error) << "Unknown tree node type";
        throw invalid_argument("Unknown tree node type");
      }
  }
  return val;
}

/* Get sysrepo subtree data corresponding to XPATH */
string JsonEncode::read(string xpath)
{
  sysrepo::S_Tree sr_tree;
  Json::StyledWriter styledwriter; //pretty JSON
  Json::FastWriter fastWriter; //unreadable JSON
  string prettyJson;
  Json::Value val;

  if (xpath.back() == '.' || xpath.back() == '*') {
    /* XPATH identify multiple instances */
    throw invalid_argument("get xpaths must not end with '.' or '*'");
    // could be retrieved with get_subtrees(xpath.c_str())
  }

  BOOST_LOG_TRIVIAL(debug) << "read and encode in json data for " << xpath;
  /* XPATH identify a single instance */
  sr_tree = sr_sess->get_subtree(xpath.c_str());
    if (sr_tree == nullptr)
      throw invalid_argument("xpath not found");

  //// Print in sysrepo tree format
  //BOOST_LOG_TRIVIAL(debug) << "\n" << sr_tree->to_string(10);

  val = json_tree(sr_tree);

  /* Print Pretty JSON message */
  prettyJson = styledwriter.write(val);
  BOOST_LOG_TRIVIAL(debug) << prettyJson;

  return fastWriter.write(val); /* return Fast unreadable JSON message */
}
