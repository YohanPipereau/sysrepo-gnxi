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

#ifndef _ENCODE_H
#define _ENCODE_H

#include <libyang/Libyang.hpp>
#include <sysrepo-cpp/Session.hpp>

#include <jsoncpp/json/json.h>

using std::shared_ptr;
using std::string;
using std::vector;

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

struct JsonData {
  JsonData() {}
  /* Field containing a YANG list key [name=value] */
  std::pair<string, string> key;
  /* Field containing the JSON tree under the designed YANG element */
  string data;
};

/*
 * Factory to instantiate encodings
 * Encoding can be {JSON, Bytes, Proto, ASCII, JSON_IETF}
 */
class Encode {
  public:
    Encode(std::shared_ptr<sysrepo::Session> sr_sess);
    ~Encode();

    /* Supported Encodings */
    enum Supported {
      JSON_IETF = 0,
    };

    /* JSON encoding */
    void json_update(string data);
    vector<JsonData> json_read(string xpath);

  private:
    void storeTree(libyang::S_Data_Node node);
    void storeLeaf(libyang::S_Data_Node_Leaf_List node);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
    sysrepo::S_Subscribe sub; //must be out of constructor to recv callback
};

#endif //_ENCODE_H
