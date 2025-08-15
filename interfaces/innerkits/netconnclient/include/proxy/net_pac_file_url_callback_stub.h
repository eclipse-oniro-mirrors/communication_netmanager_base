/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef NET_PAC_FILE_URL_CALLBACK_STUB_H
#define NET_PAC_FILE_URL_CALLBACK_STUB_H

#include <map>

#include "iremote_stub.h"

#include "i_net_pac_file_url_callback.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

class NetPacFileUrlCallbackStub : public IRemoteStub<INetPacFileUrlCallback> {
 public:
  NetPacFileUrlCallbackStub();
  virtual ~NetPacFileUrlCallbackStub() = default;

  int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

  int32_t PacFileUrlChange(const std::string &pacFileUrl) override;

 private:
  using NetPacFileUrlCallbackFunc = int32_t (NetPacFileUrlCallbackStub::*)(MessageParcel &, MessageParcel &);
  int32_t PacFileUrlChange(MessageParcel &data, MessageParcel &reply);
  std::map<uint32_t, NetPacFileUrlCallbackFunc> memberFuncMap_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_PAC_FILE_URL_CALLBACK_STUB_H
