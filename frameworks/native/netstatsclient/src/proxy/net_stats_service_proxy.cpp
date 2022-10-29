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

#include "net_stats_service_proxy.h"

#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsServiceProxy::NetStatsServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<INetStatsService>(impl) {}

NetStatsServiceProxy::~NetStatsServiceProxy() = default;

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
        return ERR_NULL_OBJECT;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR);
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NSM_REGISTER_NET_STATS_CALLBACK, dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetStatsServiceProxy::UnregisterNetStatsCallback(const sptr<INetStatsCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return ERR_NULL_OBJECT;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return static_cast<int32_t>(NetStatsResultCode::ERR_INTERNAL_ERROR);
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NSM_UNREGISTER_NET_STATS_CALLBACK, dataParcel, replyParcel, option);
    if (retCode != ERR_NONE) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int64_t NetStatsServiceProxy::GetIfaceRxBytes(const std::string &interfaceName)
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }
    if (!data.WriteString(interfaceName)) {
        NETMGR_LOG_E("WriteString failed");
        return err;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_GET_IFACE_RXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetIfaceTxBytes(const std::string &interfaceName)
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }
    if (!data.WriteString(interfaceName)) {
        NETMGR_LOG_E("WriteString failed");
        return err;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_GET_IFACE_TXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetCellularRxBytes()
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }
    MessageParcel reply;
    MessageOption option;
    int error = remote->SendRequest(CMD_GET_CELLULAR_RXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetCellularTxBytes()
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }
    MessageParcel reply;
    MessageOption option;
    int error = remote->SendRequest(CMD_GET_CELLULAR_TXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetAllRxBytes()
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }
    MessageParcel reply;
    MessageOption option;
    int error = remote->SendRequest(CMD_GET_ALL_RXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetAllTxBytes()
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }
    MessageParcel reply;
    MessageOption option;
    int error = remote->SendRequest(CMD_GET_ALL_TXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetUidRxBytes(uint32_t uid)
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }
    if (!data.WriteUint32(uid)) {
        NETMGR_LOG_E("proxy uid%{public}d", uid);
        return err;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }

    MessageParcel reply;
    MessageOption option;
    int error = remote->SendRequest(CMD_GET_UID_RXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}

int64_t NetStatsServiceProxy::GetUidTxBytes(uint32_t uid)
{
    int64_t err = -1;
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return err;
    }
    if (!data.WriteUint32(uid)) {
        NETMGR_LOG_E("proxy uid%{public}d", uid);
        return err;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return err;
    }

    MessageParcel reply;
    MessageOption option;
    int error = remote->SendRequest(CMD_GET_UID_TXBYTES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return err;
    }
    return reply.ReadInt64();
}
} // namespace NetManagerStandard
} // namespace OHOS
