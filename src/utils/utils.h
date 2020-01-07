/*
 * Copyright 2020 Yohan Pipereau
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <chrono>
#include <string>

using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

/* Get current time since epoch in nanosec */
inline uint64_t get_time_nanosec()
{
  nanoseconds ts;
  ts = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

  return ts.count();
}

/* Conversion methods between xpaths and gNMI paths */
inline string gnmi_to_xpath(const Path& path)
{
  string str = "";
  bool first = true;

  if (path.elem_size() <= 0)
    return str;

  //iterate over the list of PathElem of a gNMI path
  for (auto &node : path.elem()) {
    str += "/";
    if (first) {
      first = false;
      /* YANG namespace is specified in origin field */
      if (!path.origin().empty())
        str += path.origin() + ":";
    }

    str += node.name();
    for (auto key : node.key()) //0 or 1 iteration
      str += "[" + key.first + "=\"" + key.second + "\"]";
  }

  return str;
}

#endif // _UTILS_H
