/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "net_policy_callback_stub.h"

#include "net_mgr_log_wrapper.h"
#include "net_policy_constants.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyCallbackStub::NetPolicyCallbackStub()
{
    memberFuncMap_[NOTIFY_NET_UID_POLICY_CHANGE] = &NetPolicyCallbackStub::OnNetUidPolicyChange;
    memberFuncMap_[NOTIFY_NET_UID_RULE_CHANGE] = &NetPolicyCallbackStub::OnNetUidRuleChange;
    memberFuncMap_[NOTIFY_NET_QUOTA_POLICY_CHANGE] = &NetPolicyCallbackStub::OnNetQuotaPolicyChange;
    memberFuncMap_[NOTIFY_NET_METERED_IFACES_CHANGE] = &NetPolicyCallbackStub::OnNetMeteredIfacesChange;
    memberFuncMap_[NOTIFY_BACKGROUND_POLICY_CHANGE] = &NetPolicyCallbackStub::OnNetBackgroundPolicyChange;
}

NetPolicyCallbackStub::~NetPolicyCallbackStub() {}

int32_t NetPolicyCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                               MessageOption &option)
{
    std::u16string myDescripter = NetPolicyCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_E("Descriptor checked failed");
        return ERR_FLATTEN_OBJECT;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetPolicyCallbackStub::OnNetUidPolicyChange(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("ReadUint32 uid failed");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t policy = 0;
    if (!data.ReadUint32(policy)) {
        NETMGR_LOG_E("ReadUint32 policy failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetUidPolicyChange(uid, policy);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::NetUidRuleChange(uint32_t uid, uint32_t rule)
{
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::OnNetUidRuleChange(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("ReadUint32 uid failed");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t rule = 0;
    if (!data.ReadUint32(rule)) {
        NETMGR_LOG_E("ReadUint32 rule failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetUidRuleChange(uid, rule);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::NetUidPolicyChange(uint32_t uid, uint32_t policy)
{
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::OnNetBackgroundPolicyChange(MessageParcel &data, MessageParcel &reply)
{
    bool isBackgroundPolicyAllow = false;
    if (!data.ReadBool(isBackgroundPolicyAllow)) {
        NETMGR_LOG_E("ReadBool isBackgroundPolicyAllow failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetBackgroundPolicyChange(isBackgroundPolicyAllow);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::NetBackgroundPolicyChange(bool isBackgroundPolicyAllow)
{
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::OnNetQuotaPolicyChange(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetQuotaPolicy> cellularPolicies;
    if (!NetQuotaPolicy::Unmarshalling(data, cellularPolicies)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetQuotaPolicyChange(cellularPolicies);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::NetQuotaPolicyChange(const std::vector<NetQuotaPolicy> &cellularPolicies)
{
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::OnNetMeteredIfacesChange(MessageParcel &data, MessageParcel &reply)
{
    uint32_t size;
    std::vector<std::string> ifaces;

    if (!data.ReadUint32(size)) {
        NETMGR_LOG_E("ReadUInt32Vector size failed");
        return ERR_FLATTEN_OBJECT;
    }
    for (uint32_t i = 0; i < size; ++i) {
        std::string iface;
        if (!data.ReadString(iface)) {
            NETMGR_LOG_E("ReadString ifaces failed");
            return ERR_FLATTEN_OBJECT;
        }
        ifaces.push_back(iface);
    }
    int32_t result = NetMeteredIfacesChange(ifaces);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::NetMeteredIfacesChange(std::vector<std::string> &ifaces)
{
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::OnNetStrategySwitch(MessageParcel &data, MessageParcel &reply)
{
    std::string iccid;
    if (!data.ReadString(iccid)) {
        return ERR_FLATTEN_OBJECT;
    }

    bool enable = false;
    if (!data.ReadBool(enable)) {
        return ERR_FLATTEN_OBJECT;
    }

    int32_t result = NetStrategySwitch(iccid, enable);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }
    return ERR_NONE;
}

int32_t NetPolicyCallbackStub::NetStrategySwitch(const std::string &iccid, bool enable)
{
    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
