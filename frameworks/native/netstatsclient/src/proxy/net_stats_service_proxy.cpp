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
#include "net_stats_service_proxy.h"

#include "net_stats_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsServiceProxy::NetStatsServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetStatsService>(impl)
{}

NetStatsServiceProxy::~NetStatsServiceProxy() {}

int32_t NetStatsServiceProxy::SystemReady()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_SYSTEM_READY, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
    }
    return error;
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

NetStatsResultCode NetStatsServiceProxy::GetIfaceStatsDetail(const std::string &iface, uint32_t start,
    uint32_t end, NetStatsInfo &statsInfo)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    NETMGR_LOG_D("proxy iface[%{public}s], start[%{public}d], end[%{public}d]",
        iface.c_str(), start, end);
    if (!data.WriteString(iface)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(start)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(end)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_GET_IFACE_STATS_DETAIL, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!NetStatsInfo::Unmarshalling(reply, statsInfo)) {
        NETMGR_LOG_E("NetStatsInfo Unmarshalling failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return static_cast<NetStatsResultCode>(reply.ReadInt32());
}

NetStatsResultCode NetStatsServiceProxy::GetUidStatsDetail(const std::string &iface, uint32_t uid,
    uint32_t start, uint32_t end, NetStatsInfo &statsInfo)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    NETMGR_LOG_D("proxy iface[%{public}s], uid[%{public}d],start[%{public}d], end[%{public}d]",
        iface.c_str(), uid, start, end);
    if (!data.WriteString(iface)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(uid)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(start)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(end)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_GET_UID_STATS_DETAIL, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    if (!NetStatsInfo::Unmarshalling(reply, statsInfo)) {
        NETMGR_LOG_E("NetStatsInfo Unmarshalling failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return static_cast<NetStatsResultCode>(reply.ReadInt32());
}

NetStatsResultCode NetStatsServiceProxy::UpdateIfacesStats(const std::string &iface,
    uint32_t start, uint32_t end, const NetStatsInfo &stats)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    NETMGR_LOG_D("proxy iface[%{public}s]", iface.c_str());
    if (!data.WriteString(iface) || !data.WriteUint32(start) || !data.WriteUint32(end)) {
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    if (!stats.Marshalling(data)) {
        NETMGR_LOG_E("Proxy Marshalling failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_UPDATE_IFACES_STATS, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return static_cast<NetStatsResultCode>(reply.ReadInt32());
}

NetStatsResultCode NetStatsServiceProxy::UpdateStatsData()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_UPDATE_STATS_DATA, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return static_cast<NetStatsResultCode>(reply.ReadInt32());
}

NetStatsResultCode NetStatsServiceProxy::ResetFactory()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NSM_RESET_FACTORY, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetStatsResultCode::ERR_INTERNAL_ERROR;
    }
    return static_cast<NetStatsResultCode>(reply.ReadInt32());
}
} // namespace NetManagerStandard
} // namespace OHOS
