/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <stdexcept>

#include "utils.h"

using namespace std;
using libyang::Data_Node;
using libyang::S_Data_Node;

libyang::S_Data_Node
XpathParser::create_ly_node(sysrepo::S_Tree tree, libyang::S_Data_Node parent,
                         libyang::S_Module module, const char *node_name)
{
  /* Create the new node */
  switch (tree->type()) {
    case SR_STRING_T:
    case SR_BINARY_T:
    case SR_BITS_T:
    case SR_ENUM_T:
    case SR_IDENTITYREF_T:
    case SR_INSTANCEID_T:
    case SR_LEAF_EMPTY_T:
    case SR_BOOL_T:
    case SR_DECIMAL64_T:
    case SR_UINT8_T:
    case SR_UINT16_T:
    case SR_UINT32_T:
    case SR_UINT64_T:
    case SR_INT8_T:
    case SR_INT16_T:
    case SR_INT32_T:
    case SR_INT64_T:
        //Create new leaf
        cout << "DEBUG: " << __FUNCTION__ << " LEAF" << endl;
        return make_shared<Data_Node>(parent, module, node_name,
                           tree->value_to_string().c_str());
    case SR_ANYDATA_T:
    case SR_ANYXML_T:
        //create new ANYDATA/ANYXML Data Node
        cout << "DEBUG: " << __FUNCTION__ << " ANY" << endl;
        return make_shared<Data_Node>(parent, module, node_name,
                           tree->value_to_string().c_str(), LYD_ANYDATA_SXML);
    case SR_LIST_T:
    case SR_CONTAINER_T:
    case SR_CONTAINER_PRESENCE_T:
        //Create new Container/list Data Node
        cout << "DEBUG: " << __FUNCTION__ << " CONTAINER" << endl;
        return make_shared<Data_Node>(parent, module, node_name);
    default:
        cerr << "ERROR: Unknown" << endl;
        return nullptr;
    }
}

void
XpathParser::build_tree(sysrepo::S_Tree tree, libyang::S_Data_Node node)
{
  libyang::S_Module module;
  libyang::S_Data_Node tmp;
  sysrepo::S_Tree iter;

  //recursion stops when sysrepo tree node has no more children
  if (tree->first_child() == nullptr)
    return;

  // run through all siblings
  for (iter = tree->first_child(); iter != nullptr; iter = iter->next()) {
    //get module of current sibling
    module = ctx->get_module(iter->module_name());
    if (module == nullptr) {
      cerr << "ERROR: module not found" << endl;
      return;
    }

    //create sibling with "node" as a parent
    tmp = create_ly_node(iter, node, module, iter->name());
    if (tmp == nullptr)
      return;

    build_tree(iter, tmp);
  }
}

/*
 * Given a YANG XPATH, return the Data Tree node.
 */
libyang::S_Data_Node
XpathParser::create_ly_tree(sysrepo::S_Tree tree)
{
  libyang::S_Data_Node root;
  libyang::S_Module module;

  /* Create root node of libyang subtree */
  //1. Get libyang Module
  module = ctx->get_module(tree->module_name());
  if (module == nullptr) {
    cerr << "ERROR: module not found" << endl;
    return nullptr;
  }
  //2. create root node
  root = create_ly_node(tree, nullptr, module, tree->name());

  build_tree(tree, root);

  return root;
}
