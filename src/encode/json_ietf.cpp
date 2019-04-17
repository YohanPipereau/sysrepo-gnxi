/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <exception>

#include <sysrepo-cpp/Struct.hpp>
#include <libyang/Tree_Schema.hpp>
#include <libyang/Tree_Data.hpp>

#include "json_ietf.h"

using namespace std;
using namespace libyang;

using sysrepo::Val;
using sysrepo::S_Val;

void Json::print_loaded_module()
{
  cout << "=================================================="
       << endl;
  for (auto it : ctx->get_module_iter())
    cout << string(it->name()) << endl;
  cout << "=================================================="
       << endl;
}

/*
 * @brief Fetch all modules implemented in sysrepo datastore
 */
Json::Json(std::shared_ptr<sysrepo::Session> sr_sess)
  : sr_sess(sr_sess)
{
  shared_ptr<sysrepo::Yang_Schemas> schemas; //sysrepo YANG schemas supported
  sysrepo::Subscribe sub(sr_sess); //sysrepo subscriptions
  shared_ptr<ModuleCallback> scb(new ModuleCallback());
  S_Module mod;
  string str;

  /* 1. build libyang context */
  ctx = make_shared<Context>();

  /* 2. get the list of schemas from sysrepo */
  try {
    schemas = sr_sess->list_schemas();
  } catch (const exception &exc) {
    cerr << "ERROR: " << __FILE__ << " l." << __LINE__ << exc.what() << endl;
    exit(1);
  }

  /* 3.1 Callback for missing modules */
  auto mod_c_cb = [sr_sess, this](const char *mod_name, const char *mod_rev,
    const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        string str; S_Module mod;

        cout << "Importing missing dependency " << string(mod_name) << endl;
        str = sr_sess->get_schema(mod_name, mod_rev, NULL, SR_SCHEMA_YANG);

        try {
          mod = this->ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
        } catch (const exception &exc) {
          cerr << "WARN: " << __FILE__
               << " l." << __LINE__ << exc.what()
               << endl;
        }

        print_loaded_module();
        return {LYS_IN_YANG, mod_name};
    };

  /* 3.2 register callback for missing YANG module */
  ctx->add_missing_module_callback(mod_c_cb);

  /* 4. use modules from sysrepo */
  for (unsigned int i = 0; i < schemas->schema_cnt(); i++) {
    std::string module_name = schemas->schema(i)->module_name();
    std::string revision = schemas->schema(i)->revision()->revision();

    mod = ctx->get_module(module_name.c_str(), revision.c_str());
    if (mod != nullptr) {
      cout << "Module was already loaded: "
           << module_name << "@" << revision
           << endl;
      continue;
    }

    cout << "Download & parse module: "
         << module_name << "@" << revision
         << endl;

    //Download YANG model form sysrepo as a string in YANG format and parse it
    try {
      str = sr_sess->get_schema(module_name.c_str(), revision.c_str(), NULL,
                                SR_SCHEMA_YANG);
      mod = ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
    } catch (const exception &exc) {
      cerr << "WARN: " << __FILE__
           << " l." << __LINE__ << " " << exc.what()
           << endl;
    }

  }

  /* 5. subscribe for notifications about new modules */
  sub.module_install_subscribe(scb);

  /* 6. subscribe for changes of features state */
  sub.feature_enable_subscribe(scb);
}

/*
 * Wrapper to test wether the current Data Node is a key.
 * We know that by looking in the Schema Tree.
 * @param leaf Leaf Data Node
 */
bool isKey(S_Data_Node_Leaf_List leaf)
{
  S_Schema_Node_Leaf tmp = make_shared<Schema_Node_Leaf>(leaf->schema());

  if (tmp->is_key())
    return true;
  else
    return false;
}

void Json::setAtomic(libyang::S_Data_Node_Leaf_List node)
{
  shared_ptr<Val> sval;

  if (isKey(node)) {
    /* If node is a key create it first by setting parent path */
    cout << "leaf key: " << node->path() << endl;
    return;
  } else {
    cout << "leaf: " << node->path() << endl;
  }

  switch(node->value_type()) {
    case LY_TYPE_BINARY:        /* Any binary data */
      cout << "DEBUG binary: " << node->value()->binary() << endl;
      sval = make_shared<Val>(node->value()->binary(), SR_STRING_T);
      break;
    case LY_TYPE_STRING:        /* Human-readable string */
      cout << "DEBUG string: " << node->value()->string() << endl;
      sval = make_shared<Val>(node->value()->string(), SR_STRING_T);
      break;
    case LY_TYPE_BOOL:          /* "true" or "false" */
      cout << "DEBUG bool: " << static_cast<bool>(node->value()->bln()) << endl;
      sval = make_shared<Val>(static_cast<bool>(node->value()->bln()));
      break;
    case LY_TYPE_DEC64:         /* 64-bit signed decimal number */
      cout << "DEBUG dec64: " << node->value()->dec64() << endl;
      sval = make_shared<Val>(static_cast<double>(node->value()->dec64()));
    case LY_TYPE_INT8:          /* 8-bit signed integer */
      cout << "DEBUG int8: " << node->value()->int8() << endl;
      sval = make_shared<Val>(node->value()->int8());
      break;
    case LY_TYPE_UINT8:         /* 8-bit unsigned integer */
      cout << "DEBUG uint8: " << node->value()->uint8() << endl;
      sval = make_shared<Val>(node->value()->uint8());
      break;
    case LY_TYPE_INT16:         /* 16-bit signed integer */
      cout << "DEBUG int16: " << node->value()->int16() << endl;
      sval = make_shared<Val>(node->value()->int16());
      break;
    case LY_TYPE_UINT16:        /* 16-bit unsigned integer */
      cout << "DEBUG uint16: " << node->value()->uint16() << endl;
      sval = make_shared<Val>(node->value()->uint16());
      break;
    case LY_TYPE_INT32:         /* 32-bit signed integer */
      cout << "DEBUG int32: " << node->value()->int32() << endl;
      sval = make_shared<Val>(node->value()->int32());
      break;
    case LY_TYPE_UINT32:        /* 32-bit unsigned integer */
      cout << "DEBUG uint32: " << node->value()->uintu32() << endl;
      sval = make_shared<Val>(node->value()->uintu32());
      break;
    case LY_TYPE_INT64:         /* 64-bit signed integer */
      cout << "DEBUG int64: " << node->value()->int64() << endl;
      sval = make_shared<Val>(node->value()->int64(), SR_INT64_T);
      break;
    case LY_TYPE_UINT64:        /* 64-bit unsigned integer */
      cout << "DEBUG uint64: " << node->value()->uint64() << endl;
      sval = make_shared<Val>(node->value()->uint64());
      break;
    case LY_TYPE_IDENT:         /* A reference to an abstract identity */
    {
      string str(node->value()->ident()->module()->name());
      str.append(":");
      str.append(node->value()->ident()->name());
      cout << "DEBUG identityref: " << str << endl;
      sval = make_shared<Val>(str.c_str(), SR_IDENTITYREF_T);
      break;
    }
    case LY_TYPE_ENUM:          /* Enumerated strings */
      cout << "DEBUG enum: " << node->value()->enm()->name() << endl;
      sval = make_shared<Val>(node->value()->enm()->name(), SR_ENUM_T);
      break;
    case LY_TYPE_EMPTY:         /* A leaf that does not have any value */
      cout << "DEBUG EMPTY LEAF: " << endl;
      sval = make_shared<Val>(nullptr, SR_LEAF_EMPTY_T);
      break;
    case LY_TYPE_LEAFREF:       /* A reference to a leaf instance */
    {
      //run again this function
      S_Data_Node_Leaf_List leaf
        = make_shared<Data_Node_Leaf_List>(node->value()->leafref());

      setAtomic(leaf);
      break;
    }

/* Unsupported types */
    case LY_TYPE_BITS:          /* A set of bits or flags */
      cerr << "WARN" << "Unsupported BITS type" << endl;
      throw std::invalid_argument("Unsupported BITS type");
      break;
    case LY_TYPE_INST:          /* References a data tree node */
      cerr << "WARN" << "Unsupported INSTANCE-IDENTIFIER type" << endl;
      throw std::invalid_argument("Unsupported INSTANCE-IDENTIFIER type");
      break;
    case LY_TYPE_UNION:         /* Choice of member types */
      cerr << "WARN" << "Unsupported UNION type" << endl;
      throw std::invalid_argument("Unsupported UNION type");
      break;
    case LY_TYPE_DER:           /* Derived type */
      cerr << "WARN" << "Unsupported DERIVED type" << endl;
      throw std::invalid_argument("Unsupported DERIVED type");
      break;
    case LY_TYPE_UNKNOWN:       /* Unknown type (used in edit-config leaves) */
      cerr << "WARN" << "Unsupported UNKNOWN type" << endl;
      throw std::invalid_argument("Unsupported UNKNOWN type");
      break;
    default:
      cerr << "WARN" << "UNKNOWN type" << endl;
      throw std::invalid_argument("Unknown type");
  }

  try {
    sr_sess->set_item(node->path().c_str(), sval);
  } catch (exception &exc) {
    cerr << "WARN: " << __FILE__
         << " l." << __LINE__
         << " " << exc.what() << endl;
    throw; //rethrow as caught
  }
}


/* Parse a message encoded in JSON IETF
 * @param data Input data encoded in JSON
 */
void Json::set(string data)
{
  S_Data_Node node;

  /* Parse input JSON, same options than netopeer2 edit-config */
  node = ctx->parse_data_mem(data.c_str(), LYD_JSON, LYD_OPT_EDIT |
                                                     LYD_OPT_STRICT);
  for (auto it : node->tree_dfs()) {
    /* Run through the entire tree, including siblinigs */

    switch(it->schema()->nodetype()) {
      case LYS_LEAF: //Only LEAF & LEAF LIST hold values in sysrepo
        {
          S_Data_Node_Leaf_List itleaf = make_shared<Data_Node_Leaf_List>(it);

          try {
            setAtomic(itleaf);
          } catch (std::string str) { //triggered by sysepo::Val constructor
            cerr << "ERROR:" << str << endl;
            throw invalid_argument("Internal error with JSON encoding");
          }
        break;
        }

      case LYS_LEAFLIST: //Only LEAF & LEAF LIST hold values in sysrepo
        cout << "leaf-list: " << it->path() << endl;
        break;
        //TODO all leaves hav the same type, but there can be multiple
        //Are they all read-only?

      case LYS_LIST: //A list instance must be created before populating leaves
        {
          cout << "list: " << it->path() << endl;
          try {
            shared_ptr<Val> sval = make_shared<Val>(nullptr, SR_LIST_T);
            sr_sess->set_item(it->path().c_str(), sval);
          } catch (exception &exc) {
            cerr << "WARN: " << __FILE__
              << " l." << __LINE__
              << " " << exc.what() << endl;
            throw; //rethrow as caught
          }

          break;
        }

      default:
        break;
    }
  }
}
