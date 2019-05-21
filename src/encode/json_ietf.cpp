/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <exception>

#include <sysrepo-cpp/Struct.hpp>
#include <sysrepo-cpp/Sysrepo.hpp>
#include <libyang/Tree_Schema.hpp>
#include <libyang/Tree_Data.hpp>

#include "encode.h"

#include <utils/log.h>

using namespace std;
using namespace libyang;

using sysrepo::Val;
using sysrepo::S_Val;
using sysrepo::S_Iter_Value;

/*****************
 * CRUD - UPDATE *
 *****************/

/*
 * Wrapper to test wether the current Data Node is a key.
 * We know that by looking in the Schema Tree.
 * @param leaf Leaf Data Node
 */
static bool isKey(S_Data_Node_Leaf_List leaf)
{
  S_Schema_Node_Leaf tmp = make_shared<Schema_Node_Leaf>(leaf->schema());

  if (tmp->is_key())
    return true;
  else
    return false;
}

/*
 * Store YANG leaf in sysrepo datastore
 * @param node Describe a libyang Data Tree leaf or leaf list
 */
void Encode::storeLeaf(libyang::S_Data_Node_Leaf_List node)
{
  shared_ptr<Val> sval;

  if (isKey(node)) {
    /* If node is a key create it first by setting parent path */
    BOOST_LOG_TRIVIAL(debug) << "leaf key: " << node->path();
    return;
  } else {
    BOOST_LOG_TRIVIAL(debug) << "leaf: " << node->path();
  }

  switch(node->value_type()) {
    case LY_TYPE_BINARY:        /* Any binary data */
      sval = make_shared<Val>(node->value()->binary(), SR_STRING_T);
      break;
    case LY_TYPE_STRING:        /* Human-readable string */
      sval = make_shared<Val>(node->value()->string(), SR_STRING_T);
      break;
    case LY_TYPE_BOOL:          /* "true" or "false" */
      sval = make_shared<Val>(static_cast<bool>(node->value()->bln()));
      break;
    case LY_TYPE_DEC64:         /* 64-bit signed decimal number */
      sval = make_shared<Val>(static_cast<double>(node->value()->dec64()));
      break;
    case LY_TYPE_INT8:          /* 8-bit signed integer */
      sval = make_shared<Val>(node->value()->int8(), SR_INT8_T);
      //sval = make_shared<Val>(node->value()->int8());
      break;
    case LY_TYPE_UINT8:         /* 8-bit unsigned integer */
      sval = make_shared<Val>(node->value()->uint8(), SR_UINT8_T);
      //sval = make_shared<Val>(node->value()->uint8());
      break;
    case LY_TYPE_INT16:         /* 16-bit signed integer */
      sval = make_shared<Val>(node->value()->int16(), SR_INT16_T);
      //sval = make_shared<Val>(node->value()->int16());
      break;
    case LY_TYPE_UINT16:        /* 16-bit unsigned integer */
      sval = make_shared<Val>(node->value()->uint16(), SR_UINT16_T);
      //sval = make_shared<Val>(node->value()->uint16());
      break;
    case LY_TYPE_INT32:         /* 32-bit signed integer */
      sval = make_shared<Val>(node->value()->int32(), SR_INT32_T);
      //sval = make_shared<Val>(node->value()->int32());
      break;
    case LY_TYPE_UINT32:        /* 32-bit unsigned integer */
      sval = make_shared<Val>(node->value()->uintu32(), SR_UINT32_T);
      //sval = make_shared<Val>(node->value()->uintu32());
      break;
    case LY_TYPE_INT64:         /* 64-bit signed integer */
      sval = make_shared<Val>(node->value()->int64(), SR_INT64_T);
      break;
    case LY_TYPE_UINT64:        /* 64-bit unsigned integer */
      sval = make_shared<Val>(node->value()->uint64(), SR_UINT64_T);
      //sval = make_shared<Val>(node->value()->uint64());
      break;
    case LY_TYPE_IDENT:         /* A reference to an abstract identity */
    {
      string str(node->value()->ident()->module()->name());
      str.append(":");
      str.append(node->value()->ident()->name());
      sval = make_shared<Val>(str.c_str(), SR_IDENTITYREF_T);
      break;
    }
    case LY_TYPE_ENUM:          /* Enumerated strings */
      sval = make_shared<Val>(node->value()->enm()->name(), SR_ENUM_T);
      break;
    case LY_TYPE_EMPTY:         /* A leaf that does not have any value */
      sval = make_shared<Val>(nullptr, SR_LEAF_EMPTY_T);
      break;
    case LY_TYPE_LEAFREF:       /* A reference to a leaf instance */
    {
      //run again this function
      S_Data_Node_Leaf_List leaf
        = make_shared<Data_Node_Leaf_List>(node->value()->leafref());
      storeLeaf(leaf);
      break;
    }

/* Unsupported types */
    case LY_TYPE_BITS:          /* A set of bits or flags */
      BOOST_LOG_TRIVIAL(warning) << "Unsupported BITS type";
      throw std::invalid_argument("Unsupported BITS type");
      break;
    case LY_TYPE_INST:          /* References a data tree node */
      BOOST_LOG_TRIVIAL(warning) << "Unsupported INSTANCE-IDENTIFIER type" << endl;
      throw std::invalid_argument("Unsupported INSTANCE-IDENTIFIER type");
      break;
    case LY_TYPE_UNION:         /* Choice of member types */
      BOOST_LOG_TRIVIAL(warning) << "Unsupported UNION type";
      throw std::invalid_argument("Unsupported UNION type");
      break;
    case LY_TYPE_DER:           /* Derived type */
      BOOST_LOG_TRIVIAL(warning) << "Unsupported DERIVED type";
      throw std::invalid_argument("Unsupported DERIVED type");
      break;
    case LY_TYPE_UNKNOWN:       /* Unknown type (used in edit-config leaves) */
      BOOST_LOG_TRIVIAL(warning) << "Unsupported UNKNOWN type";
      throw std::invalid_argument("Unsupported UNKNOWN type");
      break;
    default:
      BOOST_LOG_TRIVIAL(warning) << "UNKNOWN type";
      throw std::invalid_argument("Unknown type");
  }

  try {
    sr_sess->set_item(node->path().c_str(), sval);
  } catch (exception &exc) {
    BOOST_LOG_TRIVIAL(warning) << exc.what();
    throw; //rethrow as caught
  }
}

void Encode::storeTree(libyang::S_Data_Node node)
{
  for (auto it : node->tree_dfs()) {
    /* Run through the entire tree, including siblinigs */

    switch(it->schema()->nodetype()) {
      case LYS_LEAF: //Only LEAF & LEAF LIST hold values in sysrepo
        {
          S_Data_Node_Leaf_List itleaf = make_shared<Data_Node_Leaf_List>(it);

          try {
            storeLeaf(itleaf);
          } catch (std::string str) { //triggered by sysepo::Val constructor
            BOOST_LOG_TRIVIAL(error) << str;
            throw invalid_argument("Internal error with JSON encoding");
          }
          break;
        }

      case LYS_LEAFLIST: //Only LEAF & LEAF LIST hold values in sysrepo
        BOOST_LOG_TRIVIAL(warning) << "Unsupported leaf-list: " << it->path();
        break;
        //TODO all leaves hav the same type, but there can be multiple
        //Are they all read-only?

      case LYS_LIST: //A list instance must be created before populating leaves
        {
          try {
            shared_ptr<Val> sval = make_shared<Val>(nullptr, SR_LIST_T);
            sr_sess->set_item(it->path().c_str(), sval);
          } catch (exception &exc) {
            BOOST_LOG_TRIVIAL(warning) << exc.what();
            throw; //rethrow as caught
          }

          break;
        }

      default:
        break;
    }
  }
}

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

Json::Value
JsonEncode::json_tree(sysrepo::S_Tree tree)
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
  std::string prettyJson;
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
