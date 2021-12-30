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

#ifndef NET_POLICY_SERVICE_STUB_H
#define NET_POLICY_SERVICE_STUB_H

#include <map>

#include "iremote_stub.h"

#include "i_net_policy_service.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyServiceStub : public IRemoteStub<INetPolicyService> {
public:
    NetPolicyServiceStub();
    ~NetPolicyServiceStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using NetPolicyServiceFunc = int32_t (NetPolicyServiceStub::*)(MessageParcel &, MessageParcel &);

private:
    int32_t OnSetUidPolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetUidPolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetUids(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsUidNetAccessMetered(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsUidNetAccessIfaceName(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetPolicyCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnregisterNetPolicyCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetNetPolicys(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetNetPolicys(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetCellularPolicys(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetCellularPolicys(MessageParcel &data, MessageParcel &reply);
    int32_t OnResetFactory(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetBackgroundPolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetBackgroundPolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetBackgroundPolicyByUid(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetCurrentBackgroundPolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnSnoozePolicy(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetIdleWhitelist(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetIdleWhitelist(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, NetPolicyServiceFunc> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_SERVICE_STUB_H
