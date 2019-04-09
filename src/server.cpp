/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <chrono>

#include "server.h"

using namespace chrono;


uint64_t GNMIServer::get_time_nanosec()
{
  nanoseconds ts;
  ts = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

  return ts.count();
}

string GNMIServer::gnmi_to_xpath(const Path& path)
{
  string str = "";

  if (path.elem_size() <= 0)
    return str;

  //iterate over the list of PathElem of a gNMI path
  for (auto &node : path.elem()) {
    str += "/" + node.name();
    for (auto key : node.key()) //0 or 1 iteration
      str += "[" + key.first + "=\"" + key.second + "\"]";
  }

  return str;
}

void GNMIServer::xpath_to_gnmi(Path *path, string xpath)
{
  //TODO
  return;
}
