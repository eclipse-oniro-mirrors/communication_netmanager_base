/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_CALLBACK_STUB_H
#define NET_CONN_CALLBACK_STUB_H

#include <map>

#include "iremote_stub.h"

#include "i_net_conn_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnCallbackStub : public IRemoteStub<INetConnCallback> {
public:
    NetConnCallbackStub();
    virtual ~NetConnCallbackStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using NetConnCallbackFunc = int32_t (NetConnCallbackStub::*)(MessageParcel &, MessageParcel &);

private:
    int32_t OnNetConnStateChanged(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetAvailable(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetCapabilitiesChange(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetConnectionPropertiesChange(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetLost(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, NetConnCallbackFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_CALLBACK_STUB_H
