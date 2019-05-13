/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_H
#define _ENCODE_H

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

#include "../proto/gnmi.pb.h" //for gnmi::Encoding

using std::shared_ptr;

/*
 * Encode directory aims at providing a CRUD wrapper on top of sysrepo
 * for JSON encoding (other encodings can be added).
 * It provides YANG validation before storing elements and after fetching them
 * in sysrepo.
 *
 * -update()  CREATE & UPDATE
 * -read()    READ //TODO
 * -delete()  DELETE //TODO
 *
 */

/*
 * Abstract Top class for Encodings
 * All encodings inherits from this class which also provide helpers
 * specific
 */
class Encode {
  public:
    Encode(std::shared_ptr<sysrepo::Session> sess) : sr_sess(sess) {}
    virtual void update(std::string data) = 0;

  protected:
    void storeTree(libyang::S_Data_Node node);
    void storeLeaf(libyang::S_Data_Node_Leaf_List node);

  protected:
    std::shared_ptr<sysrepo::Session> sr_sess;
};

/* Class for JSON IETF encoding */
class Json : public Encode {
  public:
    Json(shared_ptr<libyang::Context> lctx,shared_ptr<sysrepo::Session> sess)
        : Encode(sess), ctx(lctx) {}
    void update(std::string data) override;

  private:
    std::shared_ptr<libyang::Context> ctx;
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
