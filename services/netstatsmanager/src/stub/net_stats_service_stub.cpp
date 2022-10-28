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

#include "net_stats_service_stub.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsServiceStub::NetStatsServiceStub()
{
    memberFuncMap_[CMD_NSM_REGISTER_NET_STATS_CALLBACK] = &NetStatsServiceStub::OnRegisterNetStatsCallback;
    memberFuncMap_[CMD_NSM_UNREGISTER_NET_STATS_CALLBACK] = &NetStatsServiceStub::OnUnregisterNetStatsCallback;
    memberFuncMap_[CMD_GET_IFACE_RXBYTES] = &NetStatsServiceStub::OnGetIfaceRxBytes;
    memberFuncMap_[CMD_GET_IFACE_TXBYTES] = &NetStatsServiceStub::OnGetIfaceTxBytes;
    memberFuncMap_[CMD_GET_CELLULAR_RXBYTES] = &NetStatsServiceStub::OnGetCellularRxBytes;
    memberFuncMap_[CMD_GET_CELLULAR_TXBYTES] = &NetStatsServiceStub::OnGetCellularTxBytes;
    memberFuncMap_[CMD_GET_ALL_RXBYTES] = &NetStatsServiceStub::OnGetAllRxBytes;
    memberFuncMap_[CMD_GET_ALL_TXBYTES] = &NetStatsServiceStub::OnGetAllTxBytes;
    memberFuncMap_[CMD_GET_UID_RXBYTES] = &NetStatsServiceStub::OnGetUidRxBytes;
    memberFuncMap_[CMD_GET_UID_TXBYTES] = &NetStatsServiceStub::OnGetUidTxBytes;
}

NetStatsServiceStub::~NetStatsServiceStub() = default;

int32_t NetStatsServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                             MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescriptor = NetStatsServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        NETMGR_LOG_D("descriptor checked fail");
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

int32_t NetStatsServiceStub::OnRegisterNetStatsCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetStatsCallback> callback = iface_cast<INetStatsCallback>(remote);
    result = RegisterNetStatsCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetStatsServiceStub::OnUnregisterNetStatsCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }
    sptr<INetStatsCallback> callback = iface_cast<INetStatsCallback>(remote);
    result = UnregisterNetStatsCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetStatsServiceStub::OnGetIfaceRxBytes(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }
    int64_t result = GetIfaceRxBytes(iface);
    if (!reply.WriteInt64(result)) {
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetIfaceTxBytes(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }
    int64_t result = GetIfaceTxBytes(iface);
    if (!reply.WriteInt64(result)) {
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetCellularRxBytes(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt64(GetCellularRxBytes())) {
        NETMGR_LOG_E("WriteInt64 failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetCellularTxBytes(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt64(GetCellularTxBytes())) {
        NETMGR_LOG_E("WriteInt64 failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetAllRxBytes(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt64(GetAllRxBytes())) {
        NETMGR_LOG_E("WriteInt64 failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetAllTxBytes(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt64(GetAllTxBytes())) {
        NETMGR_LOG_E("WriteInt64 failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetUidRxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    int64_t result = GetUidRxBytes(uid);
    if (!reply.WriteInt64(result)) {
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetUidTxBytes(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    int64_t result = GetUidTxBytes(uid);
    if (!reply.WriteInt64(result)) {
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
