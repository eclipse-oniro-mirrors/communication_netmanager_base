/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NET_DIAG_CALLBACK_STUB_H
#define NET_DIAG_CALLBACK_STUB_H

#include "i_net_diag_callback.h"

#include <map>

#include "iremote_stub.h"

namespace OHOS {
namespace NetsysNative {
class NetDiagCallbackStub : public IRemoteStub<INetDiagCallback> {
public:
    NetDiagCallbackStub();
    virtual ~NetDiagCallbackStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t OnNotifyPingResult(const NetDiagPingResult &pingResult) override;

private:
    using NetDiagCallbackFunc = int32_t (NetDiagCallbackStub::*)(MessageParcel &, MessageParcel &);

private:
    int32_t CmdNotifyPingResult(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, NetDiagCallbackFunc> memberFuncMap_;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NET_DIAG_CALLBACK_STUB_H
