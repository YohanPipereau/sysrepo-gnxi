/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _RUNTIME_H
#define _RUNTIME_H

#include <sysrepo-cpp/Session.hpp>
#include <libyang/Tree_Schema.hpp>

/*
 * RuntimeSrCallback - Class defining callbacks to perform installation of
 * module, enablement of feature in sysrepo-gnxi libyang context.
 * It is triggered by sysrepo events like module installation, feature
 * enablement
 */
class RuntimeSrCallback : public sysrepo::Callback {
  public:
    RuntimeSrCallback(std::shared_ptr<libyang::Context> context,
                      std::shared_ptr<sysrepo::Session> sess)
      : ctx(context), sr_sess(sess) {}

    void module_install(const char *module_name, const char *revision,
                        sr_module_state_t state, void *private_ctx) override;

    void feature_enable(const char *module_name, const char *feature_name,
                        bool enable, void *private_ctx) override;

  private:
    void install(const char *module_name, const char *revision);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
};

#endif //_RUNTIME_H
