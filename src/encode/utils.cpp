/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <stdexcept>

#include "utils.h"

using namespace std;
using libyang::Data_Node;

/* C implementation copied from netopeer2
 *
 * valid xpaths:
 *    /node
 *    /node[pred]
 *    /module:node
 *    /module:node[pred]
 */
static const char *
_parse_node(const char *xpath, const char **mod, int *mod_len,
           const char **name, int *name_len,
           const char **pred, int *pred_len)
{
    if (xpath[0] != '/') {
        return NULL;
    }

    /* parse module name */
    *mod = xpath + 1;
    *mod_len = 0;
    while (((*mod)[*mod_len] != ':') && ((*mod)[*mod_len] != '/') && ((*mod)[*mod_len] != '[')
            && ((*mod)[*mod_len] != '\0')) {
        ++(*mod_len);
    }
    if ((*mod)[*mod_len] != ':') {
        /* no module name, this is the node name */
        *name = *mod;
        *mod = NULL;
        *name_len = *mod_len;
        *mod_len = 0;
    } else {
        /* parse node name */
        *name = *mod + *mod_len + 1;
        *name_len = 0;
        while (((*name)[*name_len] != '/') && ((*name)[*name_len] != '[') && ((*name)[*name_len] != '\0')) {
            ++(*name_len);
        }
    }

    /* parse all predicates */
    if ((*name)[*name_len] == '[') {
        *pred = *name + *name_len;
        *pred_len = 0;
        do {
            while ((*pred)[*pred_len] != ']') {
                ++(*pred_len);
            }
            ++(*pred_len);
        } while ((*pred)[*pred_len] == '[');

        xpath = *pred + *pred_len;
    } else {
        *pred = NULL;
        *pred_len = 0;

        xpath = *name + *name_len;
    }

    return xpath;
}

/*
 * C++ wrapper for _parse_node
 * TODO: remove the C function and use 100% c++
 */
shared_ptr<XpathNode> XpathParser::parse_node(string &xpath)
{
  const char *new_xpath;
  const char *mod, *node, *pred;
  int mod_len, node_len, pred_len;
  string mod_str = ""; string node_str = ""; string pred_str = "";


  if (xpath.empty())
    return nullptr;

  new_xpath = _parse_node(xpath.c_str(), &mod, &mod_len, &node, &node_len,
                          &pred, &pred_len);

  if (mod)
    mod_str = string(mod, mod_len);
  if (node)
    node_str = string(node, node_len);
  if (pred) //predicate
    pred_str = string(pred, pred_len);

  /* DO NOT create xpath string object before module/node/predicate string
   * because new_xpath and *_str var share the same memory and
   */
  if (new_xpath)
    xpath = string(new_xpath);
  else
    return nullptr; //ERROR

  return make_shared<XpathNode>(mod_str, node_str, pred_str);
}

libyang::S_Data_Node
XpathParser::create_node(sysrepo::S_Val val, libyang::S_Data_Node parent,
                         libyang::S_Module module, string node_name)
{
  /* Create the new node */
  switch (val->type()) {
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
        return make_shared<Data_Node>(parent, module, node_name.c_str(),
                           val->val_to_string().c_str());
    case SR_ANYDATA_T:
    case SR_ANYXML_T:
        //create new ANYDATA/ANYXML Data Node
        cout << "DEBUG: " << __FUNCTION__ << " ANY" << endl;
        return make_shared<Data_Node>(parent, module, node_name.c_str(),
                           val->val_to_string().c_str(), LYD_ANYDATA_SXML);
    case SR_LIST_T:
    case SR_CONTAINER_T:
    case SR_CONTAINER_PRESENCE_T:
        //Create new Container/list Data Node
        cout << "DEBUG: " << __FUNCTION__ << " CONTAINER" << endl;
        return make_shared<Data_Node>(parent, module, node_name.c_str());
    default:
        cerr << "ERROR: Unknown" << endl;
        return nullptr;
    }
}

/* Search for parent node to 'node' in cache
 * Run through all stored nodes in cache from root node to last put
 */
libyang::S_Data_Node XpathParser::search_parent_of(XpathNode node)
{
  libyang::S_Data_Node parent = nullptr;

  cout << "DEBUG: " << __FUNCTION__ << " cache size: " << cache.size() << endl;
  for (auto it : cache) {
    cout << "DEBUG: " << __FUNCTION__
         << "\n\tcache el is: " << it.first.to_string()
         << "\n\tnode is    : " << node.to_string()
         << endl;
    if (it.first == node) { //node found
      parent = it.second; //update parent with current node
      continue;
    } else { //node not found
      //Discard from this element (included) to end of cache list
      cache.erase(cache.begin(), cache.end());
      break;
    }
  }

  if (parent == nullptr)
    cout << "DEBUG: " << __FUNCTION__ << " parent not found " << endl;
  else
    cout << "DEBUG: " << __FUNCTION__ << " parent found is " << parent->path() << endl;

  return parent;
}

/*
 * Given a YANG XPATH, return the Data Tree node.
 */
libyang::S_Data_Node
XpathParser::to_lynode(sysrepo::S_Val val)
{
  shared_ptr<XpathNode> xpathNode;
  libyang::S_Data_Node node, parent;
  libyang::S_Module module;
  string xpath = val->xpath();

  if (xpath.empty()) {
    cerr << "WARN: empty xpath provided" << endl;
    return nullptr;
  }

  /* Run trough all nodes of the xpath */
  while ((xpathNode = parse_node(xpath)) != nullptr) {
    cout << "DEBUG: " << __FUNCTION__ << " xpathNode: " << xpathNode->to_string() << endl;
    parent = search_parent_of(*xpathNode);

    /* Create module, either specified else default to parent's module */
    if (xpathNode->getmodule().empty()) { //no module in node, use parent's
      if (parent == nullptr) {
        cerr << "ERROR: invalid xpath" << endl;
        return nullptr;
      } else { // use parent module
        module = parent->node_module();
        xpathNode->setmodule(parent->node_module()->name());
      }
    } else { // use module from current node
      module = ctx->get_module(xpathNode->getmodule().c_str());
    }

    /* Create node */
    if (parent)
      cout << "RUNTIME: parent " << parent->path() << endl;

    cout << "RUNTIME: node" << xpathNode->getnode() << endl;

    try {
      node = create_node(val, parent, module, xpathNode->getnode());
    } catch (exception &exc) {
      cerr << "ERROR: fail creating node: " << exc.what() << endl;
      throw;
    }
    cout << "DEBUG: " << __FUNCTION__
         << " created node is " << node->path() << endl;

    /* insert in cache */
    pair<XpathNode, libyang::S_Data_Node> pair(*xpathNode, node);
    cache.emplace_back(pair);
  }

  return node;
}
