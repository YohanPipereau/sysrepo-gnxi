/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _JSON_IETF_ENCODE_H
#define _JSON_IETF_ENCODE_H

#include <memory>
#include <iostream>

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

class Json {
  public:
    Json(std::shared_ptr<sysrepo::Session> sr_sess);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
};

#endif //_JSON_IETF_ENCODE_H
