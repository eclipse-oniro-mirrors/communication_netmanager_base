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

#include "net_stats_service_proxy.h"

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsServiceProxy::NetStatsServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetStatsService>(impl) {}

NetStatsServiceProxy::~NetStatsServiceProxy() = default;
int32_t NetStatsServiceProxy::SendRequest(sptr<IRemoteObject> &remote, uint32_t code, MessageParcel &data, MessageParcel &reply,
                                           MessageOption &option)
{
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    int32_t retCode =
        remote->SendRequest(code, data, reply, option);
    if (retCode != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    if (!reply.ReadInt32(ret)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }

    return ret;
}

bool NetStatsServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetStatsServiceProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}

int32_t NetStatsServiceProxy::RegisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    MessageOption option;
    MessageParcel replyParcel;
    return SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_REGISTER_NET_STATS_CALLBACK), dataParcel,
                            replyParcel, option);
}

int32_t NetStatsServiceProxy::UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    MessageOption option;
    MessageParcel replyParcel;
    return SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_UNREGISTER_NET_STATS_CALLBACK),
                            dataParcel, replyParcel, option);
}

int32_t NetStatsServiceProxy::GetIfaceRxBytes(uint64_t &stats, const std::string &interfaceName)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(interfaceName)) {
        NETMGR_LOG_E("WriteString failed");
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_RXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetIfaceTxBytes(uint64_t &stats, const std::string &interfaceName)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(interfaceName)) {
        NETMGR_LOG_E("WriteString failed");
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_TXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetCellularRxBytes(uint64_t &stats)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_CELLULAR_RXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetCellularTxBytes(uint64_t &stats)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_CELLULAR_TXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetAllRxBytes(uint64_t &stats)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_RXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetAllTxBytes(uint64_t &stats)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_TXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetUidRxBytes(uint64_t &stats, uint32_t uid)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint32(uid)) {
        NETMGR_LOG_E("proxy uid%{public}d", uid);
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_RXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetUidTxBytes(uint64_t &stats, uint32_t uid)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint32(uid)) {
        NETMGR_LOG_E("proxy uid%{public}d", uid);
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_TXBYTES), data, reply, option);
    if (error != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    if (!reply.ReadUint64(stats)) {
        NETMGR_LOG_E("ReadUint64 failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return error;
}

int32_t NetStatsServiceProxy::GetIfaceStatsDetail(const std::string &iface, uint64_t start, uint64_t end,
                                                  NetStatsInfo &statsInfo)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!(data.WriteString(iface) && data.WriteUint64(start) && data.WriteUint64(end))) {
        NETMGR_LOG_E("Write data failed");
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t sendResult = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_IFACE_STATS_DETAIL), data, reply, option);
    if (sendResult != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", sendResult);
        return sendResult;
    }
    if (!NetStatsInfo::Unmarshalling(reply, statsInfo)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return sendResult;
}

int32_t NetStatsServiceProxy::GetUidStatsDetail(const std::string &iface, uint32_t uid, uint64_t start, uint64_t end,
                                                NetStatsInfo &statsInfo)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!(data.WriteString(iface) && data.WriteUint32(uid) && data.WriteUint64(start) && data.WriteUint64(end))) {
        NETMGR_LOG_E("Write data failed");
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t sendResult = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_UID_STATS_DETAIL), data, reply, option);
    if (sendResult != 0) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", sendResult);
        return sendResult;
    }
    if (!NetStatsInfo::Unmarshalling(reply, statsInfo)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return sendResult;
}

int32_t NetStatsServiceProxy::UpdateIfacesStats(const std::string &iface, uint64_t start, uint64_t end,
                                                const NetStatsInfo &stats)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!(data.WriteString(iface) && data.WriteUint64(start) && data.WriteUint64(end))) {
        NETMGR_LOG_E("Write data failed");
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    if (!stats.Marshalling(data)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }

    MessageParcel reply;
    MessageOption option;
    return SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_UPDATE_IFACES_STATS), data, reply, option);
}

int32_t NetStatsServiceProxy::UpdateStatsData()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }

    MessageParcel reply;
    MessageOption option;
    return SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_UPDATE_STATS_DATA), data, reply, option);
}

int32_t NetStatsServiceProxy::ResetFactory()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    return SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_NSM_RESET_FACTORY), data, reply, option);
}

int32_t NetStatsServiceProxy::GetAllStatsInfo(std::vector<NetStatsInfo> &infos)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t result = SendRequest(remote, static_cast<uint32_t>(StatsInterfaceCode::CMD_GET_ALL_STATS_INFO), data, reply, option);
    if (result != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", result);
        return result;
    }
    if (!NetStatsInfo::Unmarshalling(reply, infos)) {
        NETMGR_LOG_E("Read stats info failed");
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return result;
}
} // namespace NetManagerStandard
} // namespace OHOS
