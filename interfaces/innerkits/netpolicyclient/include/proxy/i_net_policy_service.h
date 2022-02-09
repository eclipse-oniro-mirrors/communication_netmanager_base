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

#ifndef I_NET_POLICY_SERVICE_H
#define I_NET_POLICY_SERVICE_H

#include "iremote_broker.h"

#include "i_net_policy_callback.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_quota_policy.h"

namespace OHOS {
namespace NetManagerStandard {
class INetPolicyService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetPolicyService");
    enum {
        CMD_NSM_START = 0,
        CMD_NSM_SET_UID_POLICY = 1,
        CMD_NSM_GET_UID_POLICY = 2,
        CMD_NSM_GET_UIDS = 3,
        CMD_NSM_IS_NET_ACCESS_METERED = 4,
        CMD_NSM_IS_NET_ACCESS_IFACENAME = 5,
        CMD_NSM_REGISTER_NET_POLICY_CALLBACK = 6,
        CMD_NSM_UNREGISTER_NET_POLICY_CALLBACK = 7,
        CMD_NSM_NET_SET_QUOTA_POLICY = 8,
        CMD_NSM_NET_GET_QUOTA_POLICY = 9,
        CMD_NSM_NET_SET_CELLULAR_POLICY = 10,
        CMD_NSM_NET_GET_CELLULAR_POLICY = 11,
        CMD_NSM_FACTORY_RESET = 12,
        CMD_NSM_SNOOZE_POLICY = 13,
        CMD_NSM_SET_IDLE_TRUSTLIST = 14,
        CMD_NSM_GET_IDLE_TRUSTLIST = 15,
        CMD_NSM_SET_BACKGROUND_POLICY = 16,
        CMD_NSM_GET_BACKGROUND_POLICY = 17,
        CMD_NSM_GET_BACKGROUND_POLICY_BY_UID = 18,
        CMD_NSM_GET_BACKGROUND_POLICY_BY_CURRENT = 19,
        CMD_NSM_END = 100,
    };

public:
    virtual NetPolicyResultCode SetPolicyByUid(uint32_t uid, NetUidPolicy policy) = 0;
    virtual NetUidPolicy GetPolicyByUid(uint32_t uid) = 0;
    virtual std::vector<uint32_t> GetUidsByPolicy(NetUidPolicy policy) = 0;
    virtual bool IsUidNetAccess(uint32_t uid, bool metered) = 0;
    virtual bool IsUidNetAccess(uint32_t uid, const std::string &ifaceName) = 0;
    virtual int32_t RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) = 0;
    virtual int32_t UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) = 0;
    virtual NetPolicyResultCode SetNetQuotaPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies) = 0;
    virtual NetPolicyResultCode GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies) = 0;
    virtual NetPolicyResultCode SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies) = 0;
    virtual NetPolicyResultCode GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies) = 0;
    virtual NetPolicyResultCode SetFactoryPolicy(const std::string &simId) = 0;
    virtual NetPolicyResultCode SetBackgroundPolicy(bool backgroundPolicy) = 0;
    virtual bool GetBackgroundPolicy() = 0;
    virtual bool GetBackgroundPolicyByUid(uint32_t uid) = 0;
    virtual NetBackgroundPolicy GetCurrentBackgroundPolicy() = 0;
    virtual NetPolicyResultCode SetSnoozePolicy(int8_t netType, const std::string &simId) = 0;
    virtual NetPolicyResultCode SetIdleTrustlist(uint32_t uid, bool isTrustlist) = 0;
    virtual NetPolicyResultCode GetIdleTrustlist(std::vector<uint32_t> &uids) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_POLICY_SERVICE_H
