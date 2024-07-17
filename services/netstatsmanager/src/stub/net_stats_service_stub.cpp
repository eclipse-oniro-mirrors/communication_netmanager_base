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

#include "net_stats_service_stub.h"

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_network.h"
#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsServiceStub::NetStatsServiceStub() {}

NetStatsServiceStub::~NetStatsServiceStub() = default;

int32_t NetStatsServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                             MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripters = NetStatsServiceStub::GetDescriptor();
    std::u16string remoteDescripters = data.ReadInterfaceToken();
    if (myDescripters != remoteDescripters) {
        NETMGR_LOG_D("descriptor checked fail");
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
    }

    switch (code) {
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_REGISTER_NET_STATS_CALLBACK):
            return OnRegisterNetStatsCallback(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_UNREGISTER_NET_STATS_CALLBACK):
            return OnUnregisterNetStatsCallback(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_RXBYTES):
            return OnGetIfaceRxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_TXBYTES):
            return OnGetIfaceTxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_CELLULAR_RXBYTES):
            return OnGetCellularRxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_CELLULAR_TXBYTES):
            return OnGetCellularTxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_RXBYTES):
            return OnGetAllRxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_TXBYTES):
            return OnGetAllTxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_RXBYTES):
            return OnGetUidRxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_TXBYTES):
            return OnGetUidTxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_STATS_DETAIL):
            return OnGetIfaceStatsDetail(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_STATS_DETAIL):
            return OnGetUidStatsDetail(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_UPDATE_IFACES_STATS):
            return OnUpdateIfacesStats(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_UPDATE_STATS_DATA):
            return OnUpdateStatsData(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_RESET_FACTORY):
            return OnResetFactory(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_STATS_INFO):
            return OnGetAllStatsInfo(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_SIM_STATS_INFO):
            return OnGetAllSimStatsInfo(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_TRAFFIC_STATS_BY_NETWORK):
            return OnGetTrafficStatsByNetwork(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_TRAFFIC_STATS_BY_UID_NETWORK):
            return OnGetTrafficStatsByUidNetwork(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_SET_APP_STATS):
            return OnSetAppStats(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_COOKIE_RXBYTES):
            return OnGetCookieRxBytes(data, reply);
        case static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_COOKIE_TXBYTES):
            return OnGetCookieTxBytes(data, reply);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t NetStatsServiceStub::OnRegisterNetStatsCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    int32_t result = NETMANAGER_ERR_LOCAL_PTR_NULL;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetStatsCallback> callback = iface_cast<INetStatsCallback>(remote);
    result = RegisterNetStatsCallback(callback);
    NETMGR_LOG_D("OnRegisterNetStatsCallback result = [%{public}d]", result);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnUnregisterNetStatsCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    int32_t result = NETMANAGER_ERR_LOCAL_PTR_NULL;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }
    sptr<INetStatsCallback> callback = iface_cast<INetStatsCallback>(remote);
    result = UnregisterNetStatsCallback(callback);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t NetStatsServiceStub::OnGetIfaceRxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t stats = 0;
    std::string iface;
    if (!data.ReadString(iface)) {
        NETMGR_LOG_E("Read string failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t result = GetIfaceRxBytes(stats, iface);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetIfaceTxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t stats = 0;
    std::string iface;
    if (!data.ReadString(iface)) {
        NETMGR_LOG_E("Read string failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = GetIfaceTxBytes(stats, iface);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetCellularRxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t stats = 0;
    int32_t ret = GetCellularRxBytes(stats);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetCellularTxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t stats = 0;
    int32_t ret = GetCellularTxBytes(stats);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetAllRxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t stats = 0;
    int32_t ret = GetAllRxBytes(stats);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetAllTxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t stats = 0;
    int32_t ret = GetAllTxBytes(stats);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetUidRxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    uint64_t stats = 0;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("ReadInt32 failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = GetUidRxBytes(stats, uid);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetUidTxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    uint64_t stats = 0;
    if (!data.ReadUint32(uid)) {
        NETMGR_LOG_E("ReadInt32 failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = GetUidTxBytes(stats, uid);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetIfaceStatsDetail(MessageParcel &data, MessageParcel &reply)
{
    int32_t res = CheckNetManagerAvailable(reply);
    if (res != NETMANAGER_SUCCESS) {
        return res;
    }
    std::string iface;
    uint64_t start = 0;
    uint64_t end = 0;
    if (!(data.ReadString(iface) && data.ReadUint64(start) && data.ReadUint64(end))) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    NetStatsInfo info;
    int32_t ret = GetIfaceStatsDetail(iface, start, end, info);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!info.Marshalling(reply)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetUidStatsDetail(MessageParcel &data, MessageParcel &reply)
{
    int32_t res = CheckNetManagerAvailable(reply);
    if (res != NETMANAGER_SUCCESS) {
        return res;
    }

    std::string iface;
    uint32_t uid = 0;
    uint64_t start = 0;
    uint64_t end = 0;
    if (!(data.ReadString(iface) && data.ReadUint32(uid) && data.ReadUint64(start) && data.ReadUint64(end))) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    NetStatsInfo info;
    int32_t ret = GetUidStatsDetail(iface, uid, start, end, info);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!info.Marshalling(reply)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnUpdateIfacesStats(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    std::string iface;
    uint64_t start = 0;
    uint64_t end = 0;
    if (!(data.ReadString(iface) && data.ReadUint64(start) && data.ReadUint64(end))) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    NetStatsInfo infos;
    if (!NetStatsInfo::Unmarshalling(data, infos)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    ret = UpdateIfacesStats(iface, start, end, infos);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnUpdateStatsData(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = UpdateStatsData();
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnResetFactory(MessageParcel &data, MessageParcel &reply)
{
    int32_t res = CheckNetManagerAvailable(reply);
    if (res != NETMANAGER_SUCCESS) {
        return res;
    }

    int32_t ret = ResetFactory();
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetAllStatsInfo(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    std::vector<NetStatsInfo> infos;
    int32_t result = GetAllStatsInfo(infos);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!NetStatsInfo::Marshalling(reply, infos)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetAllSimStatsInfo(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    std::vector<NetStatsInfo> infos;
    int32_t result = GetAllSimStatsInfo(infos);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!NetStatsInfo::Marshalling(reply, infos)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetTrafficStatsByNetwork(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    sptr<NetStatsNetwork> network = NetStatsNetwork::Unmarshalling(data);
    if (network == nullptr) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    std::unordered_map<uint32_t, NetStatsInfo> infos;
    int32_t result = GetTrafficStatsByNetwork(infos, network);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!NetStatsInfo::Marshalling(reply, infos)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetTrafficStatsByUidNetwork(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    uint32_t uid = 0;
    if (!data.ReadUint32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    sptr<NetStatsNetwork> network = NetStatsNetwork::Unmarshalling(data);
    if (network == nullptr) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    std::vector<NetStatsInfoSequence> infos;
    int32_t result = GetTrafficStatsByUidNetwork(infos, uid, network);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!NetStatsInfoSequence::Marshalling(reply, infos)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnSetAppStats(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = CheckNetManagerAvailable(reply);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    PushStatsInfo info;
    if (!PushStatsInfo::Unmarshalling(data, info)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t result = SetAppStats(info);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::CheckNetManagerAvailable(MessageParcel &reply)
{
    if (!NetManagerPermission::IsSystemCaller()) {
        NETMGR_LOG_E("Permission check failed.");
        if (!reply.WriteInt32(NETMANAGER_ERR_NOT_SYSTEM_CALL)) {
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        return NETMANAGER_ERR_NOT_SYSTEM_CALL;
    }
    if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_STATS)) {
        if (!reply.WriteInt32(NETMANAGER_ERR_PERMISSION_DENIED)) {
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        return NETMANAGER_ERR_PERMISSION_DENIED;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetCookieRxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t cookie = 0;
    uint64_t stats = 0;
    if (!data.ReadUint64(cookie)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = GetCookieRxBytes(stats, cookie);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetStatsServiceStub::OnGetCookieTxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint64_t cookie = 0;
    uint64_t stats = 0;
    if (!data.ReadUint64(cookie)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = GetCookieTxBytes(stats, cookie);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("WriteInt32 failed");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint64(stats)) {
            NETMGR_LOG_E("WriteUint64 failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
