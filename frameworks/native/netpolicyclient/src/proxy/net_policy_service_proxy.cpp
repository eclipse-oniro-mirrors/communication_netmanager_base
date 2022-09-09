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

#include "net_policy_service_proxy.h"

#include "net_mgr_log_wrapper.h"
#include "net_policy_constants.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyServiceProxy::NetPolicyServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetPolicyService>(impl)
{}

NetPolicyServiceProxy::~NetPolicyServiceProxy() {}

int32_t NetPolicyServiceProxy::SetPolicyByUid(uint32_t uid, uint32_t policy)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteUint32(uid)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(policy))) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_SET_POLICY_BY_UID, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

uint32_t NetPolicyServiceProxy::GetPolicyByUid(uint32_t uid)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    if (!data.WriteUint32(uid)) {
        return NetUidPolicy::NET_POLICY_NONE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_POLICY_BY_UID, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetUidPolicy::NET_POLICY_NONE;
    }

    return reply.ReadUint32();
}

std::vector<uint32_t> NetPolicyServiceProxy::GetUidsByPolicy(uint32_t policy)
{
    MessageParcel data;
    std::vector<uint32_t> uids;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return uids;
    }

    NETMGR_LOG_D("proxy policy[%{public}d]", static_cast<uint32_t>(policy));
    if (!data.WriteUint32(static_cast<uint32_t>(policy))) {
        return uids;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return uids;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_UIDS_BY_UID, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return uids;
    }

    if (!reply.ReadUInt32Vector(&uids)) {
        NETMGR_LOG_E("proxy SendRequest Readuint32Vector failed");
    }

    return uids;
}

bool NetPolicyServiceProxy::IsUidNetAllowed(uint32_t uid, bool metered)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint32(uid)) {
        return false;
    }

    if (!data.WriteBool(metered)) {
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_IS_NET_ALLOWED_BY_METERED, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return false;
    }

    return reply.ReadBool();
}

bool NetPolicyServiceProxy::IsUidNetAllowed(uint32_t uid, const std::string &ifaceName)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint32(uid)) {
        return false;
    }

    if (!data.WriteString(ifaceName)) {
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_IS_NET_ALLOWED_BY_IFACE, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return false;
    }

    return reply.ReadBool();
}

int32_t NetPolicyServiceProxy::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return ERR_NULL_OBJECT;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NPS_REGISTER_NET_POLICY_CALLBACK, dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetPolicyServiceProxy::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return ERR_NULL_OBJECT;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NPS_UNREGISTER_NET_POLICY_CALLBACK, dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetPolicyServiceProxy::SetNetQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    MessageParcel data;
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!NetQuotaPolicy::Marshalling(data, quotaPolicies)) {
        NETMGR_LOG_E("Marshalling failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_SET_NET_QUOTA_POLICIES, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

int32_t NetPolicyServiceProxy::GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_NET_QUOTA_POLICIES, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!NetQuotaPolicy::Unmarshalling(reply, quotaPolicies)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

int32_t NetPolicyServiceProxy::ResetPolicies(const std::string &iccid)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteString(iccid)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_RESET_POLICIES, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

int32_t NetPolicyServiceProxy::SetBackgroundPolicy(bool isBackgroundPolicyAllow)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteBool(isBackgroundPolicyAllow)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_SET_BACKGROUND_POLICY, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

bool NetPolicyServiceProxy::GetBackgroundPolicy()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_BACKGROUND_POLICY, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return false;
    }

    return reply.ReadBool();
}

uint32_t NetPolicyServiceProxy::GetBackgroundPolicyByUid(uint32_t uid)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint32(uid)) {
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_BACKGROUND_POLICY_BY_UID, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return false;
    }

    return reply.ReadUint32();
}

uint32_t NetPolicyServiceProxy::GetCurrentBackgroundPolicy()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetBackgroundPolicy::NET_BACKGROUND_POLICY_NONE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetBackgroundPolicy::NET_BACKGROUND_POLICY_NONE;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_BACKGROUND_POLICY_BY_CURRENT, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetBackgroundPolicy::NET_BACKGROUND_POLICY_NONE;
    }

    return reply.ReadUint32();
}

int32_t NetPolicyServiceProxy::UpdateRemindPolicy(int32_t netType, const std::string &iccid, uint32_t remindType)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(netType)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteString(iccid)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteUint32(remindType)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_UPDATE_REMIND_POLICY, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

int32_t NetPolicyServiceProxy::SetDeviceIdleAllowedList(uint32_t uid, bool isAllowed)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteUint32(uid)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteBool(isAllowed)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_SET_IDLE_ALLOWED_LIST, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

int32_t NetPolicyServiceProxy::GetDeviceIdleAllowedList(std::vector<uint32_t> &uids)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_GET_IDLE_ALLOWED_LIST, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!reply.ReadUInt32Vector(&uids)) {
        NETMGR_LOG_E("proxy SendRequest Readuint32Vector failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

int32_t NetPolicyServiceProxy::SetDeviceIdlePolicy(bool enable)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteBool(enable)) {
        NETMGR_LOG_E("WriteBool enable failed.");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_NPS_SET_DEVICE_IDLE_POLICY, data, reply, option);
    if (retCode != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}

bool NetPolicyServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetPolicyServiceProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
