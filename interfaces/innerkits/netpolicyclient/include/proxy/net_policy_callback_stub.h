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

#ifndef NET_POLICY_CALLBACK_STUB_H
#define NET_POLICY_CALLBACK_STUB_H

#include <map>

#include "iremote_stub.h"

#include "i_net_policy_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyCallbackStub : public IRemoteStub<INetPolicyCallback> {
public:
    NetPolicyCallbackStub();
    virtual ~NetPolicyCallbackStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

public:
    int32_t NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy) override;
    int32_t NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies) override;
    int32_t NetStrategySwitch(const std::string &simId, bool enable) override;
    int32_t NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow) override;

private:
    using NetPolicyCallbackFunc = int32_t (NetPolicyCallbackStub::*)(MessageParcel &, MessageParcel &);

private:
    int32_t OnNetUidPolicyChanged(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetCellularPolicyChanged(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetStrategySwitch(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetBackgroundPolicyChanged(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, NetPolicyCallbackFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CALLBACK_STUB_H