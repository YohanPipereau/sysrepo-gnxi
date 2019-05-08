/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <exception>
#include <libyang/Tree_Schema.hpp>

#include "encode.h"

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
    cout << "[DEBUG] Module was already loaded: "
      << module_name << "@" << revision
      << endl;
    return;
  }

  /* Download module from sysrepo */
  try {
    cout << "[DEBUG] Download " << module_name << " from sysrepo" << endl;
    str = sr_sess->get_schema(module_name, revision, nullptr, SR_SCHEMA_YANG);
  } catch (const exception &exc) {
    cerr << "WARN: " << __FILE__
         << " l." << __LINE__ << " " << exc.what()
         << endl;
    return;
  }

  /* parse module */
  try {
    cout << "[DEBUG] Parse " << module_name << " with libyang" << endl;
    mod = ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
  } catch (const exception &exc) {
    cerr << "WARN: " << __FILE__
         << " l." << __LINE__ << " " << exc.what()
         << endl;
    return;
  }
}

void
ModuleCallback::module_install(const char *module_name, const char *revision,
                               sr_module_state_t state, void *private_ctx)
{
  (void)private_ctx;

  if (ctx == nullptr) {
    cout << "ERROR: Context can not be null" << endl;
    return;
  }

  switch (state) {
  case SR_MS_UNINSTALLED:
    cout << "Impossible to remove a module at runtime" << endl;
    break;

  case SR_MS_IMPORTED:
  case SR_MS_IMPLEMENTED:
    cout << "Install " << module_name << endl;
    install(module_name, revision);
    print_loaded_module(ctx);
    break;

  default:
    cerr << "ERROR: Unknown state" << endl;
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
    cerr << "ERROR: " << __FILE__ << " l." << __LINE__ << exc.what() << endl;
    exit(1);
  }

  /* 3.1 Callback for missing modules */
  auto mod_c_cb = [this](const char *mod_name, const char *mod_rev,
    const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        string str; S_Module mod;

        cout << "[DEBUG] Importing missing dependency " << mod_name << endl;
        str = this->sr_sess->get_schema(mod_name, mod_rev, NULL, SR_SCHEMA_YANG);

        try {
          mod = this->ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
        } catch (const exception &exc) {
          cerr << "WARN: " << __FILE__
               << " l." << __LINE__ << exc.what()
               << endl;
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
      cout << "[DEBUG] Module was already loaded: "
           << module_name << "@" << revision
           << endl;
    } else {
      cout << "[DEBUG] Download & parse module: "
           << module_name << "@" << revision
           << endl;

      /* 4.1 Download YANG model from sysrepo as in YANG format and parse it */
      try {
        str = sr_sess->get_schema(module_name.c_str(), revision.c_str(), NULL,
                                  SR_SCHEMA_YANG);
        mod = ctx->parse_module_mem(str.c_str(), LYS_IN_YANG);
      } catch (const exception &exc) {
        cerr << "WARN: " << __FILE__
             << " l." << __LINE__ << " " << exc.what()
             << endl;
        continue;
      }
    }

    /* 4.2 Load features loaded in sysrepo */
    for (size_t j = 0; j < schemas->schema(i)->enabled_feature_cnt(); j++) {
      string feature_name = schemas->schema(i)->enabled_features(j);

      cout << "DEBUG: " << "Loading feature " << feature_name
           << " in module " << mod->name()
           << endl;

      mod->feature_enable(feature_name.c_str());
    }
  }

  /* 5. subscribe for notifications about new modules */
  sub->module_install_subscribe(scb, ctx.get(), sysrepo::SUBSCR_DEFAULT);

  ///* 6. subscribe for changes of features state */
  //sub->feature_enable_subscribe(scb);
}

EncodeFactory::~EncodeFactory()
{
  cout << "Disconnect sysrepo session and Libyang context" << endl;
}

unique_ptr<Encode> EncodeFactory::getEncoding(gnmi::Encoding encoding) {
  if (encoding == gnmi::Encoding::JSON) {
    cout << "DEBUG: Creating JSON object" << endl;
    return unique_ptr<Encode>(new Json(ctx, sr_sess));
  } else {
    cerr << "ERROR: Unknown encoding" << endl;
    return nullptr;
  }
}
