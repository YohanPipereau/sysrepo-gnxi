/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "utils.h"

using namespace std;

string XpathParser::get_module_name(const std::string xpath)
{
  return xpath.substr(1, xpath.find(":"));
}

string XpathParser::get_last_node_name(const std::string xpath)
{
  //TODO
  return "TODO";
}
