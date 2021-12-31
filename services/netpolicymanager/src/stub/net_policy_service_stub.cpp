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
    memberFuncMap_[CMD_NSM_SET_UID_POLICY] = &NetPolicyServiceStub::OnSetUidPolicy;
    memberFuncMap_[CMD_NSM_GET_UID_POLICY] = &NetPolicyServiceStub::OnGetUidPolicy;
    memberFuncMap_[CMD_NSM_GET_UIDS] = &NetPolicyServiceStub::OnGetUids;
    memberFuncMap_[CMD_NSM_IS_NET_ACCESS_METERED] = &NetPolicyServiceStub::OnIsUidNetAccessMetered;
    memberFuncMap_[CMD_NSM_IS_NET_ACCESS_IFACENAME] = &NetPolicyServiceStub::OnIsUidNetAccessIfaceName;
    memberFuncMap_[CMD_NSM_REGISTER_NET_POLICY_CALLBACK] = &NetPolicyServiceStub::OnRegisterNetPolicyCallback;
    memberFuncMap_[CMD_NSM_UNREGISTER_NET_POLICY_CALLBACK] = &NetPolicyServiceStub::OnUnregisterNetPolicyCallback;
    memberFuncMap_[CMD_NSM_NET_SET_QUOTA_POLICY] = &NetPolicyServiceStub::OnSetNetPolicys;
    memberFuncMap_[CMD_NSM_NET_GET_QUOTA_POLICY] = &NetPolicyServiceStub::OnGetNetPolicys;
    memberFuncMap_[CMD_NSM_NET_SET_CELLULAR_POLICY] = &NetPolicyServiceStub::OnSetCellularPolicys;
    memberFuncMap_[CMD_NSM_NET_GET_CELLULAR_POLICY] = &NetPolicyServiceStub::OnGetCellularPolicys;
    memberFuncMap_[CMD_NSM_FACTORY_RESET] = &NetPolicyServiceStub::OnResetFactory;
    memberFuncMap_[CMD_NSM_SNOOZE_POLICY] = &NetPolicyServiceStub::OnSnoozePolicy;
    memberFuncMap_[CMD_NSM_SET_IDLE_WHITELIST] = &NetPolicyServiceStub::OnSetIdleWhitelist;
    memberFuncMap_[CMD_NSM_GET_IDLE_WHITELIST] = &NetPolicyServiceStub::OnGetIdleWhitelist;
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

int32_t NetPolicyServiceStub::OnSetUidPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t netPolicy;
    if (!data.ReadUint32(netPolicy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetUidPolicy(uid, static_cast<NetUidPolicy>(netPolicy))))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetUidPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(GetUidPolicy(uid)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetUids(MessageParcel &data, MessageParcel &reply)
{
    uint32_t policy;
    if (!data.ReadUint32(policy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteUInt32Vector(GetUids(static_cast<NetUidPolicy>(policy)))) {
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

int32_t NetPolicyServiceStub::OnSetNetPolicys(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;
    if (!NetPolicyQuotaPolicy::Unmarshalling(data, quotaPolicys)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetNetPolicys(quotaPolicys)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetNetPolicys(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;

    if (GetNetPolicys(quotaPolicys) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetPolicys failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetPolicyQuotaPolicy::Marshalling(reply, quotaPolicys)) {
        NETMGR_LOG_E("Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetCellularPolicys(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicys;
    if (!NetPolicyCellularPolicy::Unmarshalling(data, cellularPolicys)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetCellularPolicys(cellularPolicys)))) {
        NETMGR_LOG_E("WriteInt32 SetCellularPolicys return failed.");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetCellularPolicys(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetPolicyCellularPolicy> cellularPolicys;

    if (GetCellularPolicys(cellularPolicys) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetPolicys failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetPolicyCellularPolicy::Marshalling(reply, cellularPolicys)) {
        NETMGR_LOG_E("Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnResetFactory(MessageParcel &data, MessageParcel &reply)
{
    std::string subscrberId;
    if (!data.ReadString(subscrberId)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(ResetFactory(subscrberId)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    bool backgroundPolicy = false;
    if (!data.ReadBool(backgroundPolicy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetBackgroundPolicy(backgroundPolicy)))) {
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

    bool ret = GetBackgroundPolicyByUid(uid);
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetCurrentBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    bool ret = GetCurrentBackgroundPolicy();
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSnoozePolicy(MessageParcel &data, MessageParcel &reply)
{
    NetPolicyQuotaPolicy quotaPolicy;
    if (!NetPolicyQuotaPolicy::Unmarshalling(data, quotaPolicy)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SnoozePolicy(quotaPolicy)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnSetIdleWhitelist(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    bool isWhiteList = false;
    if (!data.ReadBool(isWhiteList)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetIdleWhitelist(uid, isWhiteList)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetIdleWhitelist(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> uids;
    if (GetIdleWhitelist(uids) != NetPolicyResultCode::ERR_NONE) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteUInt32Vector(uids)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
