/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_UTILS_H
#define _ENCODE_UTILS_H

#include <list>
#include <libyang/Tree_Data.hpp>
#include <sysrepo-cpp/Struct.hpp>

/**
 * XpathNode -
 */
class XpathNode {
  public:
    XpathNode(std::string mod, std::string nod = "", std::string pred = "")
      : module(mod), node(nod), predicate(pred) {}
    ~XpathNode() {}
    /* const getters */
    std::string getmodule() const {return module;}
    std::string getnode() const {return node;}
    std::string getpredicate() const {return predicate;}
    /* setters */
    void setmodule(std::string module) {this->module = module;};
    void setnode(std::string node) {this->node = node;};
    void setpredicate(std::string predicate) {this->predicate = predicate;};

    std::string to_string() {
      return "mod=" + this->getmodule()
             + " node=" + this->getnode() + " "
             + " predicate=" + this->getpredicate();
    }

    inline bool operator==(const XpathNode& rhs)
    {
      return this->getmodule() == rhs.getmodule()
          && this->getnode() == rhs.getnode()
          && this->getpredicate() == rhs.getpredicate();
    }

  private:
    std::string module;
    std::string node;
    std::string predicate;
};


/**
 * XpathCache - Map caching Data tree Node previously populated
 */
typedef std::list<std::pair<XpathNode, libyang::S_Data_Node>> XpathCache;
typedef std::list<std::pair<XpathNode, libyang::S_Data_Node>> XpathCache;

/**
 * XpathParser -
 */
class XpathParser {
  public:
    XpathParser(libyang::S_Context lctx) : ctx(lctx) {}
    ~XpathParser() {}
    /* convert sysrepo Value to libyang Data Node */
    libyang::S_Data_Node to_lynode(sysrepo::S_Val val);

  private:
    std::shared_ptr<XpathNode> parse_node(std::string xpath);
    libyang::S_Data_Node search_parent_of(XpathNode node);
    libyang::S_Data_Node create_node(sysrepo::S_Val val,
                                     libyang::S_Data_Node parent,
                                     libyang::S_Module module,
                                     std::string node_name);

  private:
    XpathCache cache;
    libyang::S_Context ctx;
};

#endif // _ENCODE_UTILS_H
