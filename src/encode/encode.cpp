/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <exception>
#include <libyang/Tree_Schema.hpp>

#include "encode.h"
#include <utils/log.h>

using namespace std;
using namespace libyang;

static void print_loaded_module(std::shared_ptr<libyang::Context> ctx)
{
  cout << "=================================================="
       << endl;
  for (auto it : ctx->get_module_iter())
    cout << string(it->name()) << endl;
  cout << "=================================================="
       << endl;
}

/* Class defining callbacks when a module or a feature is loaded in sysrepo */
class ModuleCallback : public sysrepo::Callback {
  public:
    ModuleCallback(shared_ptr<libyang::Context> context,
                   shared_ptr<sysrepo::Session> sess)
      : ctx(context), sr_sess(sess) {}

    void module_install(const char *module_name, const char *revision,
                        sr_module_state_t state, void *private_ctx) override;

    //void feature_enable(const char *module_name, const char *feature_name,
    //                    bool enable, void *private_ctx) override
    //{
    //    cout << "My callback" << endl;
    ////  (void)private_ctx;
    ////  //TODO feature loaded in sysrepo after sysrepo-gnmi has been ran
    ////  cout << "New feature has been installed"
    ////            << string(module_name)
    ////            << string(feature_name) << endl;
    //}

  private:
    void uninstall();
    void install(const char *module_name, const char *revision);

  private:
    shared_ptr<libyang::Context> ctx;
    shared_ptr<sysrepo::Session> sr_sess;
};

void ModuleCallback::install(const char *module_name, const char *revision)
{
  libyang::S_Module mod;
  string str;

  /* Is module already loaded with libyang? */
  mod = ctx->get_module(module_name, revision);
  if (mod != nullptr) {
    BOOST_LOG_TRIVIAL(debug) << "Module was already loaded: "
                             << module_name << "@" << revision;
    return;
  }

  /* Download module from sysrepo */
  try {
    BOOST_LOG_TRIVIAL(debug) << "Download " << module_name << " from sysrepo";
    str = sr_sess->get_schema(module_name, revision, nullptr, SR_SCHEMA_YANG);
  } catch (const exception &exc) {
    BOOST_LOG_TRIVIAL(warning) << exc.what();
    return;
  }

  /* parse module */
  try {
    BOOST_LOG_TRIVIAL(debug) << "Parse " << module_name << " with libyang";
    mod = ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
  } catch (const exception &exc) {
    BOOST_LOG_TRIVIAL(warning) << exc.what();
    return;
  }
}

void
ModuleCallback::module_install(const char *module_name, const char *revision,
                               sr_module_state_t state, void *private_ctx)
{
  (void)private_ctx;

  if (ctx == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Context can not be null";
    return;
  }

  switch (state) {
  case SR_MS_UNINSTALLED:
    BOOST_LOG_TRIVIAL(warning) << "Impossible to remove a module at runtime";
    break;

  case SR_MS_IMPORTED:
  case SR_MS_IMPLEMENTED:
    BOOST_LOG_TRIVIAL(info) << "Install " << module_name;
    install(module_name, revision);
    print_loaded_module(ctx);
    break;

  default:
    BOOST_LOG_TRIVIAL(error) << "Unknown state";
  }
}

/*
 * @brief Fetch all modules implemented in sysrepo datastore
 */
EncodeFactory::EncodeFactory(shared_ptr<sysrepo::Session> sess)
  : sr_sess(sess)
{
  shared_ptr<sysrepo::Yang_Schemas> schemas; //sysrepo YANG schemas supported
  shared_ptr<ModuleCallback> scb; //pointer to callback class
  sub = make_shared<sysrepo::Subscribe>(sr_sess); //sysrepo subscriptions
  S_Module mod;
  string str;

  /* 1. build libyang context */
  ctx = make_shared<Context>();

  /* Instantiate Callback class */
  scb = make_shared<ModuleCallback>(ctx, sess);

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

  /* 4. use modules from sysrepo */
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

  ///* 6. subscribe for changes of features state */
  //TODO sub->feature_enable_subscribe(scb);
}

EncodeFactory::~EncodeFactory()
{
  BOOST_LOG_TRIVIAL(info) << "Disconnect sysrepo session and Libyang context";
}

unique_ptr<Encode> EncodeFactory::getEncoding(EncodeFactory::Encoding encoding)
{
  switch (encoding) {
    case EncodeFactory::Encoding::JSON_IETF:
      return unique_ptr<Encode>(new JsonEncode(ctx, sr_sess));

    default:
      BOOST_LOG_TRIVIAL(error) << "Unknown encoding";
      return nullptr;
  }
}
