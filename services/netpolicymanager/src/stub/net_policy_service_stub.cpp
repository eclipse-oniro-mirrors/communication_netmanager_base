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
#include "net_policy_service_stub.h"

#include "net_policy_cellular_policy.h"
#include "net_policy_quota_policy.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyServiceStub::NetPolicyServiceStub()
{
    memberFuncMap_[CMD_NSM_SET_UID_POLICY] = &NetPolicyServiceStub::OnSetPolicyByUid;
    memberFuncMap_[CMD_NSM_GET_UID_POLICY] = &NetPolicyServiceStub::OnGetPolicyByUid;
    memberFuncMap_[CMD_NSM_GET_UIDS] = &NetPolicyServiceStub::OnGetUidsByPolicy;
    memberFuncMap_[CMD_NSM_IS_NET_ACCESS_METERED] = &NetPolicyServiceStub::OnIsUidNetAccessMetered;
    memberFuncMap_[CMD_NSM_IS_NET_ACCESS_IFACENAME] = &NetPolicyServiceStub::OnIsUidNetAccessIfaceName;
    memberFuncMap_[CMD_NSM_REGISTER_NET_POLICY_CALLBACK] = &NetPolicyServiceStub::OnRegisterNetPolicyCallback;
    memberFuncMap_[CMD_NSM_UNREGISTER_NET_POLICY_CALLBACK] = &NetPolicyServiceStub::OnUnregisterNetPolicyCallback;
    memberFuncMap_[CMD_NSM_NET_SET_QUOTA_POLICY] = &NetPolicyServiceStub::OnSetNetQuotaPolicies;
    memberFuncMap_[CMD_NSM_NET_GET_QUOTA_POLICY] = &NetPolicyServiceStub::OnGetNetQuotaPolicies;
    memberFuncMap_[CMD_NSM_NET_SET_CELLULAR_POLICY] = &NetPolicyServiceStub::OnSetCellularPolicies;
    memberFuncMap_[CMD_NSM_NET_GET_CELLULAR_POLICY] = &NetPolicyServiceStub::OnGetCellularPolicies;
    memberFuncMap_[CMD_NSM_FACTORY_RESET] = &NetPolicyServiceStub::OnSetFactoryPolicy;
    memberFuncMap_[CMD_NSM_SNOOZE_POLICY] = &NetPolicyServiceStub::OnSnoozePolicy;
    memberFuncMap_[CMD_NSM_SET_IDLE_TRUSTLIST] = &NetPolicyServiceStub::OnSetIdleTrustlist;
    memberFuncMap_[CMD_NSM_GET_IDLE_TRUSTLIST] = &NetPolicyServiceStub::OnGetIdleTrustlist;
    memberFuncMap_[CMD_NSM_SET_BACKGROUND_POLICY] = &NetPolicyServiceStub::OnSetBackgroundPolicy;
    memberFuncMap_[CMD_NSM_GET_BACKGROUND_POLICY] = &NetPolicyServiceStub::OnGetBackgroundPolicy;
    memberFuncMap_[CMD_NSM_GET_BACKGROUND_POLICY_BY_UID] = &NetPolicyServiceStub::OnGetBackgroundPolicyByUid;
    memberFuncMap_[CMD_NSM_GET_BACKGROUND_POLICY_BY_CURRENT] = &NetPolicyServiceStub::OnGetCurrentBackgroundPolicy;
}

NetPolicyServiceStub::~NetPolicyServiceStub() {}

int32_t NetPolicyServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescripter = NetPolicyServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_E("descriptor checked fail");
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

int32_t NetPolicyServiceStub::OnSetPolicyByUid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t netPolicy;
    if (!data.ReadUint32(netPolicy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetPolicyByUid(uid, static_cast<NetUidPolicy>(netPolicy))))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetPolicyByUid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(GetPolicyByUid(uid)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetUidsByPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t policy;
    if (!data.ReadUint32(policy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteUInt32Vector(GetUidsByPolicy(static_cast<NetUidPolicy>(policy)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnIsUidNetAccessMetered(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    bool metered = false;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadBool(metered)) {
        return ERR_FLATTEN_OBJECT;
    }

    bool ret = IsUidNetAccess(uid, metered);
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnIsUidNetAccessIfaceName(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    std::string ifaceName;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadString(ifaceName)) {
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = IsUidNetAccess(uid, ifaceName);
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnRegisterNetPolicyCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetPolicyCallback> callback = iface_cast<INetPolicyCallback>(remote);
    result = RegisterNetPolicyCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetPolicyServiceStub::OnUnregisterNetPolicyCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }
    sptr<INetPolicyCallback> callback = iface_cast<INetPolicyCallback>(remote);
    result = UnregisterNetPolicyCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetPolicyServiceStub::OnSetNetQuotaPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;
    if (!NetPolicyQuotaPolicy::Unmarshalling(data, quotaPolicies)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetNetQuotaPolicies(quotaPolicies)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetNetQuotaPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;

    if (GetNetQuotaPolicies(quotaPolicies) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetQuotaPolicies failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetPolicyQuotaPolicy::Marshalling(reply, quotaPolicies)) {
        NETMGR_LOG_E("Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetCellularPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicies;
    if (!NetPolicyCellularPolicy::Unmarshalling(data, cellularPolicies)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetCellularPolicies(cellularPolicies)))) {
        NETMGR_LOG_E("WriteInt32 SetCellularPolicies return failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetCellularPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicies;

    if (GetCellularPolicies(cellularPolicies) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetQuotaPolicies failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetPolicyCellularPolicy::Marshalling(reply, cellularPolicies)) {
        NETMGR_LOG_E("Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetFactoryPolicy(MessageParcel &data, MessageParcel &reply)
{
    std::string subscrberId;
    if (!data.ReadString(subscrberId)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetFactoryPolicy(subscrberId)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    bool isBackgroundPolicyAllow = false;
    if (!data.ReadBool(isBackgroundPolicyAllow)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetBackgroundPolicy(isBackgroundPolicyAllow)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    bool ret = GetBackgroundPolicy();
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetBackgroundPolicyByUid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteBool(GetBackgroundPolicyByUid(uid))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetCurrentBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt32(static_cast<int32_t>(GetCurrentBackgroundPolicy()))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSnoozePolicy(MessageParcel &data, MessageParcel &reply)
{
    int8_t netType = 0;
    if (!data.ReadInt8(netType)) {
        return ERR_FLATTEN_OBJECT;
    }

    std::string simId;
    if (!data.ReadString(simId)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetSnoozePolicy(netType, simId)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetIdleTrustlist(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    bool isTrustlist = false;
    if (!data.ReadBool(isTrustlist)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetIdleTrustlist(uid, isTrustlist)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetIdleTrustlist(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> uids;
    if (GetIdleTrustlist(uids) != NetPolicyResultCode::ERR_NONE) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteUInt32Vector(uids)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
