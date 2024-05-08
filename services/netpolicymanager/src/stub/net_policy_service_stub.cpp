/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "net_mgr_log_wrapper.h"
#include "net_policy_core.h"
#include "net_quota_policy.h"
#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
std::map<uint32_t, const char *> g_codeNPS = {
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POLICY_BY_UID), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_POLICY_BY_UID), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_UIDS_BY_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_IS_NET_ALLOWED_BY_METERED), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_IS_NET_ALLOWED_BY_IFACE), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_REGISTER_NET_POLICY_CALLBACK), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_UNREGISTER_NET_POLICY_CALLBACK),
     Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NET_QUOTA_POLICIES), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_NET_QUOTA_POLICIES), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_UPDATE_REMIND_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_IDLE_TRUSTLIST), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_IDLE_TRUSTLIST), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_DEVICE_IDLE_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_RESET_POLICIES), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_BACKGROUND_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_BACKGROUND_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_BACKGROUND_POLICY_BY_UID), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POWER_SAVE_TRUSTLIST), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_POWER_SAVE_TRUSTLIST), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POWER_SAVE_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_CHECK_PERMISSION), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NETWORK_ACCESS_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_NETWORK_ACCESS_POLICY), Permission::MANAGE_NET_STRATEGY},
    {static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_NOTIFY_NETWORK_ACCESS_POLICY_DIAG),
     Permission::MANAGE_NET_STRATEGY},
};
} // namespace

NetPolicyServiceStub::NetPolicyServiceStub()
{
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POLICY_BY_UID)] =
        &NetPolicyServiceStub::OnSetPolicyByUid;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_POLICY_BY_UID)] =
        &NetPolicyServiceStub::OnGetPolicyByUid;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_UIDS_BY_POLICY)] =
        &NetPolicyServiceStub::OnGetUidsByPolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_IS_NET_ALLOWED_BY_METERED)] =
        &NetPolicyServiceStub::OnIsUidNetAllowedMetered;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_IS_NET_ALLOWED_BY_IFACE)] =
        &NetPolicyServiceStub::OnIsUidNetAllowedIfaceName;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_REGISTER_NET_POLICY_CALLBACK)] =
        &NetPolicyServiceStub::OnRegisterNetPolicyCallback;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_UNREGISTER_NET_POLICY_CALLBACK)] =
        &NetPolicyServiceStub::OnUnregisterNetPolicyCallback;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NET_QUOTA_POLICIES)] =
        &NetPolicyServiceStub::OnSetNetQuotaPolicies;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_NET_QUOTA_POLICIES)] =
        &NetPolicyServiceStub::OnGetNetQuotaPolicies;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_RESET_POLICIES)] =
        &NetPolicyServiceStub::OnResetPolicies;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_UPDATE_REMIND_POLICY)] =
        &NetPolicyServiceStub::OnSnoozePolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_IDLE_TRUSTLIST)] =
        &NetPolicyServiceStub::OnSetDeviceIdleTrustlist;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_IDLE_TRUSTLIST)] =
        &NetPolicyServiceStub::OnGetDeviceIdleTrustlist;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_DEVICE_IDLE_POLICY)] =
        &NetPolicyServiceStub::OnSetDeviceIdlePolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_POWER_SAVE_TRUSTLIST)] =
        &NetPolicyServiceStub::OnGetPowerSaveTrustlist;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POWER_SAVE_TRUSTLIST)] =
        &NetPolicyServiceStub::OnSetPowerSaveTrustlist;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_BACKGROUND_POLICY)] =
        &NetPolicyServiceStub::OnSetBackgroundPolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_BACKGROUND_POLICY)] =
        &NetPolicyServiceStub::OnGetBackgroundPolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_BACKGROUND_POLICY_BY_UID)] =
        &NetPolicyServiceStub::OnGetBackgroundPolicyByUid;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POWER_SAVE_POLICY)] =
        &NetPolicyServiceStub::OnSetPowerSavePolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_CHECK_PERMISSION)] =
        &NetPolicyServiceStub::OnCheckPermission;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_FACTORYRESET_POLICIES)] =
        &NetPolicyServiceStub::OnFactoryResetPolicies;
    ExtraNetPolicyServiceStub();
    InitEventHandler();
}

void NetPolicyServiceStub::ExtraNetPolicyServiceStub()
{
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NETWORK_ACCESS_POLICY)] =
        &NetPolicyServiceStub::OnSetNetworkAccessPolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_NETWORK_ACCESS_POLICY)] =
        &NetPolicyServiceStub::OnGetNetworkAccessPolicy;
    memberFuncMap_[static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_NOTIFY_NETWORK_ACCESS_POLICY_DIAG)] =
        &NetPolicyServiceStub::OnNotifyNetAccessPolicyDiag;
    return;
}

NetPolicyServiceStub::~NetPolicyServiceStub() = default;

void NetPolicyServiceStub::InitEventHandler()
{
    runner_ = AppExecFwk::EventRunner::Create(NET_POLICY_WORK_THREAD);
    if (!runner_) {
        NETMGR_LOG_E("Create net policy work event runner.");
        return;
    }
    auto core = DelayedSingleton<NetPolicyCore>::GetInstance();
    handler_ = std::make_shared<NetPolicyEventHandler>(runner_, core);
    core->Init(handler_);
}

int32_t NetPolicyServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                              MessageOption &option)
{
    std::u16string myDescriptor = NetPolicyServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        NETMGR_LOG_E("descriptor checked fail");
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
    }
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);
    if (handler_ == nullptr) {
        NETMGR_LOG_E("Net policy handler is null, recreate handler.");
        InitEventHandler();
        if (handler_ == nullptr) {
            NETMGR_LOG_E("recreate net policy handler failed.");
            return NETMANAGER_ERR_INTERNAL;
        }
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        int32_t checkPermissionResult = CheckPolicyPermission(code);
        if (checkPermissionResult != NETMANAGER_SUCCESS) {
            if (!reply.WriteInt32(checkPermissionResult)) {
                return IPC_STUB_WRITE_PARCEL_ERR;
            }
            return NETMANAGER_SUCCESS;
        }
        int32_t result = NETMANAGER_SUCCESS;
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            handler_->PostSyncTask(
                [this, &data, &reply, &requestFunc, &result]() { result = (this->*requestFunc)(data, reply); },
                AppExecFwk::EventQueue::Priority::HIGH);
            NETMGR_LOG_D("stub call end, code = [%{public}d], ret = [%{public}d]", code, result);
            return result;
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

bool NetPolicyServiceStub::SubCheckPermission(const std::string &permission, uint32_t funcCode)
{
    if (NetManagerPermission::CheckPermission(permission)) {
        return true;
    }
    NETMGR_LOG_E("Permission denied funcCode: %{public}d permission: %{public}s", funcCode, permission.c_str());
    return false;
}

int32_t NetPolicyServiceStub::CheckPolicyPermission(uint32_t code)
{
    bool result = NetManagerPermission::IsSystemCaller();
    if (!result) {
        return NETMANAGER_ERR_NOT_SYSTEM_CALL;
    }
    if (g_codeNPS.find(code) != g_codeNPS.end()) {
        result = SubCheckPermission(g_codeNPS[code], code);
        if (!result) {
            return NETMANAGER_ERR_PERMISSION_DENIED;
        }
        return NETMANAGER_SUCCESS;
    }
    NETMGR_LOG_E("Error funcCode, need check");
    return NETMANAGER_ERR_PERMISSION_DENIED;
}

int32_t NetPolicyServiceStub::OnSetPolicyByUid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("Read Uint32 data failed.");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    uint32_t netPolicy = 0;
    if (!data.ReadUint32(netPolicy)) {
        NETMGR_LOG_E("Read Uint32 data failed.");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetPolicyByUid(uid, netPolicy);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed.");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetPolicyByUid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    uint32_t policy = 0;
    int32_t result = GetPolicyByUid(uid, policy);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed.");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteInt32(policy)) {
            NETMGR_LOG_E("Write int32 reply failed.");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetUidsByPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t policy = 0;
    if (!data.ReadUint32(policy)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::vector<uint32_t> uids;
    int32_t result = GetUidsByPolicy(policy, uids);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUInt32Vector(uids)) {
            NETMGR_LOG_E("Write uint32 vector reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnIsUidNetAllowedMetered(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    bool metered = false;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (!data.ReadBool(metered)) {
        NETMGR_LOG_E("Read Bool data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    bool isAllowed = false;
    int32_t result = IsUidNetAllowed(uid, metered, isAllowed);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteBool(isAllowed)) {
            NETMGR_LOG_E("Write Bool reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnIsUidNetAllowedIfaceName(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    std::string ifaceName;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (!data.ReadString(ifaceName)) {
        NETMGR_LOG_E("Read String data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    bool isAllowed = false;
    int32_t result = IsUidNetAllowed(uid, ifaceName, isAllowed);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteBool(isAllowed)) {
            NETMGR_LOG_E("Write Bool reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnRegisterNetPolicyCallback(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        reply.WriteInt32(NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }

    sptr<INetPolicyCallback> callback = iface_cast<INetPolicyCallback>(remote);
    int32_t result = RegisterNetPolicyCallback(callback);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnUnregisterNetPolicyCallback(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    sptr<INetPolicyCallback> callback = iface_cast<INetPolicyCallback>(remote);
    int32_t result = UnregisterNetPolicyCallback(callback);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetNetQuotaPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetQuotaPolicy> quotaPolicies;
    if (!NetQuotaPolicy::Unmarshalling(data, quotaPolicies)) {
        NETMGR_LOG_E("Unmarshalling failed.");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetNetQuotaPolicies(quotaPolicies);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetNetQuotaPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::vector<NetQuotaPolicy> quotaPolicies;

    int32_t result = GetNetQuotaPolicies(quotaPolicies);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!NetQuotaPolicy::Marshalling(reply, quotaPolicies)) {
            NETMGR_LOG_E("Marshalling failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnResetPolicies(MessageParcel &data, MessageParcel &reply)
{
    std::string subscriberId;
    if (!data.ReadString(subscriberId)) {
        NETMGR_LOG_E("Read String data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = ResetPolicies(subscriberId);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    bool isBackgroundPolicyAllow = false;
    if (!data.ReadBool(isBackgroundPolicyAllow)) {
        NETMGR_LOG_E("Read Bool data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetBackgroundPolicy(isBackgroundPolicyAllow);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetBackgroundPolicy(MessageParcel &data, MessageParcel &reply)
{
    bool backgroundPolicy = false;
    int32_t result = GetBackgroundPolicy(backgroundPolicy);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteBool(backgroundPolicy)) {
            NETMGR_LOG_E("Write Bool reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetBackgroundPolicyByUid(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    uint32_t backgroundPolicyOfUid = 0;
    int32_t result = GetBackgroundPolicyByUid(uid, backgroundPolicyOfUid);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(backgroundPolicyOfUid)) {
            NETMGR_LOG_E("Write uint32 reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSnoozePolicy(MessageParcel &data, MessageParcel &reply)
{
    int32_t netType = 0;
    if (!data.ReadInt32(netType)) {
        NETMGR_LOG_E("Read int32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string simId;
    if (!data.ReadString(simId)) {
        NETMGR_LOG_E("Read String data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    uint32_t remindType = 0;
    if (!data.ReadUint32(remindType)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = UpdateRemindPolicy(netType, simId, remindType);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetDeviceIdleTrustlist(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> uids;
    if (!data.ReadUInt32Vector(&uids)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    bool isAllowed = false;
    if (!data.ReadBool(isAllowed)) {
        NETMGR_LOG_E("Read Bool data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetDeviceIdleTrustlist(uids, isAllowed);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetDeviceIdleTrustlist(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> uids;
    int32_t result = GetDeviceIdleTrustlist(uids);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUInt32Vector(uids)) {
            NETMGR_LOG_E("Write uint32 vector reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetDeviceIdlePolicy(MessageParcel &data, MessageParcel &reply)
{
    bool isAllowed = false;
    if (!data.ReadBool(isAllowed)) {
        NETMGR_LOG_E("Read Bool data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetDeviceIdlePolicy(isAllowed);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetPowerSaveTrustlist(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> uids;
    int32_t result = GetPowerSaveTrustlist(uids);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUInt32Vector(uids)) {
            NETMGR_LOG_E("Write uint32 Vector reply failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetPowerSaveTrustlist(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> uids;
    if (!data.ReadUInt32Vector(&uids)) {
        NETMGR_LOG_E("Read uint32 data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    bool isAllowed = false;
    if (!data.ReadBool(isAllowed)) {
        NETMGR_LOG_E("Read Bool data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetPowerSaveTrustlist(uids, isAllowed);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetPowerSavePolicy(MessageParcel &data, MessageParcel &reply)
{
    bool isAllowed = false;
    if (!data.ReadBool(isAllowed)) {
        NETMGR_LOG_E("Read Bool data failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = SetPowerSavePolicy(isAllowed);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnCheckPermission(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt32(NETMANAGER_SUCCESS)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnFactoryResetPolicies(MessageParcel &data, MessageParcel &reply)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnSetNetworkAccessPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;

    if (!data.ReadUint32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    uint8_t wifi_allow;
    uint8_t cellular_allow;
    NetworkAccessPolicy policy;
    bool reconfirmFlag = true;

    if (!data.ReadUint8(wifi_allow)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (!data.ReadUint8(cellular_allow)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (!data.ReadBool(reconfirmFlag)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    policy.wifiAllow = wifi_allow;
    policy.cellularAllow = cellular_allow;
    int32_t ret = SetNetworkAccessPolicy(uid, policy, reconfirmFlag);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyServiceStub::OnGetNetworkAccessPolicy(MessageParcel &data, MessageParcel &reply)
{
    int32_t uid = 0;
    uint32_t userId = 1;
    bool flag = false;
    if (!data.ReadBool(flag)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (!data.ReadInt32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (!data.ReadUint32(userId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    AccessPolicySave policies;
    AccessPolicyParameter parameters;
    parameters.flag = flag;
    parameters.uid = uid;
    parameters.userId = userId;

    int32_t ret = GetNetworkAccessPolicy(parameters, policies);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (ret == NETMANAGER_SUCCESS) {
        ret = NetworkAccessPolicy::Marshalling(reply, policies, flag);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("GetNetworkAccessPolicy marshalling failed");
            return ret;
        }
    }

    return ret;
}

int32_t NetPolicyServiceStub::OnNotifyNetAccessPolicyDiag(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_I("OnNotifyNetAccessPolicyDiag");
    uint32_t uid;

    if (!data.ReadUint32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = NotifyNetAccessPolicyDiag(uid);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("Write int32 reply failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
