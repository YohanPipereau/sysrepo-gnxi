/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <stdexcept>

#include "utils.h"

using namespace std;
using libyang::Data_Node;
using libyang::S_Data_Node;

//libyang::S_Data_Node
//XpathParser::create_node(sysrepo::S_Tree tree, libyang::S_Data_Node parent,
//                         libyang::S_Module module, string node_name)
//{
//  /* Create the new node */
//  switch (tree->type()) {
//    case SR_STRING_T:
//    case SR_BINARY_T:
//    case SR_BITS_T:
//    case SR_ENUM_T:
//    case SR_IDENTITYREF_T:
//    case SR_INSTANCEID_T:
//    case SR_LEAF_EMPTY_T:
//    case SR_BOOL_T:
//    case SR_DECIMAL64_T:
//    case SR_UINT8_T:
//    case SR_UINT16_T:
//    case SR_UINT32_T:
//    case SR_UINT64_T:
//    case SR_INT8_T:
//    case SR_INT16_T:
//    case SR_INT32_T:
//    case SR_INT64_T:
//        //Create new leaf
//        cout << "DEBUG: " << __FUNCTION__ << " LEAF" << endl;
//        return make_shared<Data_Node>(parent, module, node_name.c_str(),
//                           tree->val_to_string().c_str());
//    case SR_ANYDATA_T:
//    case SR_ANYXML_T:
//        //create new ANYDATA/ANYXML Data Node
//        cout << "DEBUG: " << __FUNCTION__ << " ANY" << endl;
//        return make_shared<Data_Node>(parent, module, node_name.c_str(),
//                           tree->val_to_string().c_str(), LYD_ANYDATA_SXML);
//    case SR_LIST_T:
//    case SR_CONTAINER_T:
//    case SR_CONTAINER_PRESENCE_T:
//        //Create new Container/list Data Node
//        cout << "DEBUG: " << __FUNCTION__ << " CONTAINER" << endl;
//        return make_shared<Data_Node>(parent, module, node_name.c_str());
//    default:
//        cerr << "ERROR: Unknown" << endl;
//        return nullptr;
//    }
//}

//libyang::S_Data_Node XpathParser::create_ly_root_node(sysrepo::S_Tree tree)
//{
//  //Get node name or root node
//
//}
//
///*
// * Given a YANG XPATH, return the Data Tree node.
// */
//libyang::S_Data_Node
//XpathParser::create_ly_tree(sysrepo::S_Tree tree)
//{
//  libyang::S_Data_Node root;
//  libyang::S_Module module;
//
//  /* Create root node of libyang subtree */
//  //1. Get libyang Module
//  module = ctx->get_module(tree->module_name().c_str());
//  if (module == nullptr) {
//    cerr << "ERROR: module not found" << endl;
//    return nullptr;
//  }
//
//  //2. Create root node
//  root = create_ly_root_node(tree);
//
//  //3.
//
//
//  root = create_ly_node(tree, module);
//
//  return root;
//}
