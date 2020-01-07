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

#ifndef _RUNTIME_H
#define _RUNTIME_H

#include <sysrepo-cpp/Session.hpp>
#include <libyang/Tree_Schema.hpp>

/*
 * RuntimeSrCallback - Class defining callbacks to perform installation of
 * module, enablement of feature in sysrepo-gnxi libyang context.
 * It is triggered by sysrepo events like module installation, feature
 * enablement
 */
class RuntimeSrCallback : public sysrepo::Callback {
  public:
    RuntimeSrCallback(std::shared_ptr<libyang::Context> context,
                      std::shared_ptr<sysrepo::Session> sess)
      : ctx(context), sr_sess(sess) {}

    void module_install(const char *module_name, const char *revision,
                        sr_module_state_t state, void *private_ctx) override;

    void feature_enable(const char *module_name, const char *feature_name,
                        bool enable, void *private_ctx) override;

  private:
    void install(const char *module_name, const char *revision);

  private:
    std::shared_ptr<libyang::Context> ctx;
    std::shared_ptr<sysrepo::Session> sr_sess;
};

#endif //_RUNTIME_H
