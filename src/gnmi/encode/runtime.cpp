/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "runtime.h"

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

/* install - download module and load it in our libyang context. */
void RuntimeSrCallback::install(const char *module_name, const char *revision)
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

/* module_install - Actions performed after sysrepo install/uninstall module
 * event. */
void
RuntimeSrCallback::module_install(const char *module_name, const char *revision,
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

void
RuntimeSrCallback::feature_enable(const char *module_name,
                                   const char *feature_name, bool enable,
                                   void *private_ctx)
{
  (void)private_ctx; (void) enable;
  BOOST_LOG_TRIVIAL(warning) << "Impossible to enable/disable feature "
                             << string(feature_name) << " of "
                             << string(module_name)
                             << " at runtime";
}

