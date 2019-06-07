/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <exception>
#include <libyang/Tree_Schema.hpp>

#include <utils/log.h>

#include "encode.h"
#include "runtime.h"

using namespace std;
using namespace libyang;

/*
 * @brief Fetch all modules implemented in sysrepo datastore
 */
Encode::Encode(shared_ptr<sysrepo::Session> sess)
  : sr_sess(sess)
{
  shared_ptr<sysrepo::Yang_Schemas> schemas; //sysrepo YANG schemas supported
  shared_ptr<RuntimeSrCallback> scb; //pointer to callback class
  sub = make_shared<sysrepo::Subscribe>(sr_sess); //sysrepo subscriptions
  S_Module mod;
  string str;

  //Libyang log level should be ERROR only
  set_log_verbosity(LY_LLERR);

  /* 1. build libyang context */
  ctx = make_shared<Context>();

  /* Instantiate Callback class */
  scb = make_shared<RuntimeSrCallback>(ctx, sess);

  /* 2. get the list of schemas from sysrepo */
  try {
    schemas = sr_sess->list_schemas();
  } catch (const exception &exc) {
    BOOST_LOG_TRIVIAL(error) << exc.what();
    exit(1);
  }

  /* 3.1 Callback for missing modules */
  auto mod_c_cb = [this](const char *mod_name, const char *mod_rev,
    const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        string str; S_Module mod;

        BOOST_LOG_TRIVIAL(debug) << "Importing missing dependency " << mod_name;
        str = this->sr_sess->get_schema(mod_name, mod_rev, NULL, SR_SCHEMA_YANG);

        try {
          mod = this->ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
        } catch (const exception &exc) {
          BOOST_LOG_TRIVIAL(warning) << exc.what();
        }

        return {LYS_IN_YANG, mod_name};
    };

  /* 3.2 register callback for missing YANG module */
  ctx->add_missing_module_callback(mod_c_cb);

  /* 4. Initialize our libyang context with modules and features
   * already loaded in sysrepo */
  for (unsigned int i = 0; i < schemas->schema_cnt(); i++) {
    string module_name = schemas->schema(i)->module_name();
    string revision = schemas->schema(i)->revision()->revision();

    mod = ctx->get_module(module_name.c_str(), revision.c_str());
    if (mod != nullptr) {
      BOOST_LOG_TRIVIAL(debug) << "Module was already loaded: "
                               << module_name << "@" << revision;
    } else {
      BOOST_LOG_TRIVIAL(debug) << "Download & parse module: "
                               << module_name << "@" << revision;

      /* 4.1 Download YANG model from sysrepo as in YANG format and parse it */
      try {
        str = sr_sess->get_schema(module_name.c_str(), revision.c_str(), NULL,
                                  SR_SCHEMA_YANG);
        mod = ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
      } catch (const exception &exc) {
        BOOST_LOG_TRIVIAL(warning) << exc.what();
        continue;
      }
    }

    /* 4.2 Load features loaded in sysrepo */
    for (size_t j = 0; j < schemas->schema(i)->enabled_feature_cnt(); j++) {
      string feature_name = schemas->schema(i)->enabled_features(j);

      BOOST_LOG_TRIVIAL(debug) << "Loading feature " << feature_name
                               << " in module " << mod->name();

      mod->feature_enable(feature_name.c_str());
    }
  }

  /* 5. subscribe for notifications about new modules */
  sub->module_install_subscribe(scb, ctx.get(), sysrepo::SUBSCR_DEFAULT);

  /* 6. subscribe for changes of features state */
  sub->feature_enable_subscribe(scb);
}

Encode::~Encode()
{
  BOOST_LOG_TRIVIAL(info) << "Disconnect sysrepo session and Libyang context";
}
