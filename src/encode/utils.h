/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#ifndef _ENCODE_UTILS_H
#define _ENCODE_UTILS_H

#include <string>

class XpathParser {
  public:
    XpathParser() {}
    ~XpathParser() {}
    std::string get_module_name(std::string xpath);
    std::string get_last_node_name(std::string xpath);

  private:
    std::string path;
};

#endif // _ENCODE_UTILS_H
