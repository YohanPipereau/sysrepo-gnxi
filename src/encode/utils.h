/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_UTILS_H
#define _ENCODE_UTILS_H

#include <libyang/Tree_Data.hpp>
#include <sysrepo-cpp/Tree.hpp>
#include <sysrepo-cpp/Struct.hpp>

/**
 * XpathParser -
 */
class XpathParser {
  public:
    XpathParser(libyang::S_Context lctx) : ctx(lctx) {}
    ~XpathParser() {}
    /* convert sysrepo Value to libyang Data Node */
    libyang::S_Data_Node create_ly_tree(sysrepo::S_Tree tree);

  private:
    void build_tree(sysrepo::S_Tree tree, libyang::S_Data_Node node);
    libyang::S_Data_Node create_ly_node(sysrepo::S_Tree tree,
                                        libyang::S_Data_Node parent,
                                        libyang::S_Module module,
                                        const char *node_name);

  private:
    libyang::S_Context ctx;
};

#endif // _ENCODE_UTILS_H
