/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _JSON_IETF_ENCODE_H
#define _JSON_IETF_ENCODE_H

#include <memory>
#include <iostream>

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

/* Class for JSON IETF encoding */
class Json {
  public:
    Json(std::shared_ptr<sysrepo::Session> sr_sess);
    void print_loaded_module();
    void set(std::string data);

  private:
    void setAtomic(libyang::S_Data_Node_Leaf_List node);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
};

/* Class defining callbacks when a module or a feature is loaded in sysrepo */
class ModuleCallback : public sysrepo::Callback {
  public:
    void module_install(const char *module_name, const char *revision,
                        sr_module_state_t state, void *private_ctx)
    {
      (void)state; (void)private_ctx;
      //TODO
      std::cout << "New module has been installed"
                << std::string(module_name)
                << std::string(revision) << std::endl;
    }

    void feature_enable(const char *module_name, const char *feature_name,
                        sr_module_state_t state, void *private_ctx)
    {
      (void)state; (void)private_ctx;
      //TODO
      std::cout << "New feature has been installed"
                << std::string(module_name)
                << std::string(feature_name) << std::endl;
    }
};

#endif //_JSON_IETF_ENCODE_H
