/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_H
#define _ENCODE_H

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

#include "../proto/gnmi.pb.h" //for gnmi::Encoding

/* Interface for Encodings */
class Encode {
  public:
    virtual void set(std::string data) = 0;
};

/* Class for JSON IETF encoding */
class Json : public Encode {
  public:
    Json(std::shared_ptr<libyang::Context> lctx,
         std::shared_ptr<sysrepo::Session> sess) : ctx(lctx), sr_sess(sess) {}
    void set(std::string data) override;

  private:
    void setAtomic(libyang::S_Data_Node_Leaf_List node);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
};

/* Factory to instantiate encodings */
class EncodeFactory {
  public:
    EncodeFactory(std::shared_ptr<sysrepo::Session> sr_sess);
    ~EncodeFactory();
    std::unique_ptr<Encode> getEncoding(gnmi::Encoding encoding);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
    sysrepo::S_Subscribe sub; //must be out of constructor to recv callback
};

#endif //_ENCODE_H
