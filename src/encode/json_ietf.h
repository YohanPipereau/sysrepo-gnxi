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
    ~Json() {std::cout << "Disconnect sysrepo session and Libyang context"
                       << std::endl;}
    void print_loaded_module();
    void set(std::string data);

  private:
    void setAtomic(libyang::S_Data_Node_Leaf_List node);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
    //must be out of constructor to recv callback
    sysrepo::S_Subscribe sub;
};

#endif //_JSON_IETF_ENCODE_H
