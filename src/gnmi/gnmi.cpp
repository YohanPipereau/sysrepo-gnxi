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

#include "gnmi.h"

#include "get.h"
#include "set.h"
#include "subscribe.h"

Status GNMIService::Set(ServerContext *context, const SetRequest* request,
                       SetResponse* response)
{
  (void)context;
  impl::Set rpc(sr_sess, encodef);

  return rpc.run(request, response);
}

Status GNMIService::Get(ServerContext *context, const GetRequest* request,
                        GetResponse* response)
{
  (void)context;
  impl::Get rpc(sr_sess, encodef);

  return rpc.run(request, response);
}

Status GNMIService::Subscribe(ServerContext* context,
                 ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeRequest request;
  impl::Subscribe rpc(sr_sess, encodef);

  return rpc.run(context, stream);

  return Status::OK;
}

