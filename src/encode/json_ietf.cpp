/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <exception>

#include <sysrepo-cpp/Struct.hpp>
#include <libyang/Tree_Schema.hpp>

#include "json_ietf.h"

using namespace std;
using namespace libyang;

void Json::print_loaded_module()
{
  for (auto it : ctx->get_module_iter()) {
    cout << string(it->name()) << endl;
  }
}

/*
 * @brief Fetch all modules implemented in sysrepo datastore
 */
Json::Json(std::shared_ptr<sysrepo::Session> sr_sess)
  : sr_sess(sr_sess)
{
  shared_ptr<sysrepo::Yang_Schemas> schemas; //sysrepo YANG schemas supported
  sysrepo::Subscribe sub(sr_sess); //sysrepo subscriptions
  S_Module mod;
  string str;

  /* 1. build libyang context */
  ctx = make_shared<Context>();

  /* 2. get the list of schemas from sysrepo */
  try {
    schemas = sr_sess->list_schemas();
  } catch (const exception &exc) {
    cerr << "ERROR:" << exc.what() << endl;
  }

  /* 3. Callback for missing modules */
  auto mod_c_cb = [sr_sess, this](const char *mod_name, const char *mod_rev,
    const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        string str; S_Module mod;

        cout << "Importing missing module " << string(mod_name) << endl;
        str = sr_sess->get_schema(mod_name, mod_rev, NULL, SR_SCHEMA_YANG);
        mod = this->ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);

        return {LYS_IN_YANG, mod_name};
    };

  /* 4. register callback for missing YANG module */
  ctx->add_missing_module_callback(mod_c_cb);

  /* 5. use modules from sysrepo */
  for (unsigned int i = 0; i < schemas->schema_cnt(); i++) {
    std::string module_name = schemas->schema(i)->module_name();
    std::string revision = schemas->schema(i)->revision()->revision();

    mod = ctx->get_module(module_name.c_str(), revision.c_str());
    if (mod != nullptr) {
      cout << "Module was already loaded: " << module_name << revision << endl;
    } else {
      cout << "Download & parse module: " << module_name << revision << endl;
      //Download YANG model form sysrepo as a string in YANG format
      try {
        str = sr_sess->get_schema(module_name.c_str(), revision.c_str(), NULL,
                                  SR_SCHEMA_YANG);
        mod = ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
      } catch (const exception &exc) {
        cerr << "ERROR:" << __FUNCTION__ << __LINE__ << exc.what() << endl;
      }
    }
  }

  /* 6. subscribe for notifications about new modules */
  //sub.module_install_subscribe();

  /* 7. subscribe for changes of features state */



}

