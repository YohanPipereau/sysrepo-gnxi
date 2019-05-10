/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_H
#define _ENCODE_H

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

#include "../proto/gnmi.pb.h" //for gnmi::Encoding

/* Interface for Encodings
 * All encodings inherits from this class to implement their set/get encoding
 * specific
 */
class Encode {
  public:
    virtual void set(std::string data) = 0;
    virtual void get(sysrepo::S_Val val) = 0;
};

/* Class for JSON IETF encoding */
class Json : public Encode {
  public:
    Json(std::shared_ptr<libyang::Context> lctx,
         std::shared_ptr<sysrepo::Session> sess) : ctx(lctx), sr_sess(sess) {}
    void set(std::string data) override;
    void get(sysrepo::S_Val val) override;

  private:
    void setAtomic(libyang::S_Data_Node_Leaf_List node);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
};

/*
 * Factory to instantiate encodings
 * Encoding can be {JSON, Bytes, Proto, ASCII, JSON_IETF}
 */
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
