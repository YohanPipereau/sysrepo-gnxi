/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_H
#define _ENCODE_H

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

#include <jsoncpp/json/json.h>

using std::shared_ptr;
using std::string;

/*
 * Encode directory aims at providing a CREATE-UPDATE-READ wrapper on top of
 * sysrepo for JSON encoding (other encodings can be added).
 * It provides YANG validation before storing elements and after fetching them
 * in sysrepo.
 *
 * -update()  CREATE & UPDATE
 * -read()    READ
 *
 * DELETE is not supported as it is not dependent of encodings.
 * Use sr_delete_item to suppress subtree from a xpath directly.
 */

/*
 * Abstract Top class for Encodings
 * All encodings inherits from this class which also provide helpers
 * specific
 */
class Encode {
  public:
    Encode(std::shared_ptr<sysrepo::Session> sess) : sr_sess(sess) {}

    virtual void update(string data) = 0;
    virtual string read(string xpath) = 0;

  protected:
    void storeTree(libyang::S_Data_Node node);
    void storeLeaf(libyang::S_Data_Node_Leaf_List node);

  protected:
    std::shared_ptr<sysrepo::Session> sr_sess;
};

/* Class for JSON IETF encoding */
class JsonEncode : public Encode {
  public:
    JsonEncode(shared_ptr<libyang::Context> lctx,
               shared_ptr<sysrepo::Session> sess) : Encode(sess), ctx(lctx) {}

    void update(string data) override;
    string read(string xpath) override;

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
    /* Supported Encodings */
    enum Encoding {
      JSON_IETF = 0,
    };
    std::unique_ptr<Encode> getEncoding(EncodeFactory::Encoding encoding);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
    sysrepo::S_Subscribe sub; //must be out of constructor to recv callback
};

#endif //_ENCODE_H
