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

#include "net_stats_service_stub.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetStatsServiceStub::NetStatsServiceStub()
{
    memberFuncMap_[CMD_SYSTEM_READY] = &NetStatsServiceStub::OnSystemReady;
    memberFuncMap_[CMD_GET_IFACE_STATS_DETAIL] = &NetStatsServiceStub::OnGetIfaceStatsDetail;
    memberFuncMap_[CMD_GET_UID_STATS_DETAIL] = &NetStatsServiceStub::OnGetUidStatsDetail;
    memberFuncMap_[CMD_UPDATE_IFACES_STATS] = &NetStatsServiceStub::OnUpdateIfacesStats;
    memberFuncMap_[CMD_UPDATE_STATS_DATA] = &NetStatsServiceStub::OnUpdateStatsData;
    memberFuncMap_[CMD_NSM_REGISTER_NET_STATS_CALLBACK] = &NetStatsServiceStub::OnRegisterNetStatsCallback;
    memberFuncMap_[CMD_NSM_UNREGISTER_NET_STATS_CALLBACK] = &NetStatsServiceStub::OnUnregisterNetStatsCallback;
    memberFuncMap_[CMD_NSM_RESET_FACTORY] = &NetStatsServiceStub::OnResetFactory;
}

NetStatsServiceStub::~NetStatsServiceStub() {}

int32_t NetStatsServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = NetStatsServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
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
    NETMGR_LOG_D("stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetStatsServiceStub::OnSystemReady(MessageParcel &data, MessageParcel &reply)
{
    SystemReady();
    return ERR_NONE;
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

int32_t NetStatsServiceStub::OnGetIfaceStatsDetail(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t start;
    if (!data.ReadUint32(start)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t end;
    if (!data.ReadUint32(end)) {
        return ERR_FLATTEN_OBJECT;
    }

    NetStatsInfo stats;
    if (GetIfaceStatsDetail(iface, start, end, stats) != NetStatsResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetIfaceStatsDetail failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetStatsInfo::Marshalling(reply, stats)) {
        NETMGR_LOG_E("proxy Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnGetUidStatsDetail(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t start;
    if (!data.ReadUint32(start)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t end;
    if (!data.ReadUint32(end)) {
        return ERR_FLATTEN_OBJECT;
    }

    NetStatsInfo stats;
    if (GetUidStatsDetail(iface, uid, start, end, stats) != NetStatsResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetUidStatsDetail failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetStatsInfo::Marshalling(reply, stats)) {
        NETMGR_LOG_E("NetStatsInfo Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnUpdateIfacesStats(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t start;
    uint32_t end;
    if (!data.ReadUint32(start) || !data.ReadUint32(end)) {
        return ERR_FLATTEN_OBJECT;
    }

    NetStatsInfo stats;
    if (!NetStatsInfo::Unmarshalling(reply, stats)) {
        NETMGR_LOG_E("NetStatsInfo::Unmarshalling failed");
    }
    int32_t ret = static_cast<int32_t>(UpdateIfacesStats(iface, start, end, stats));
    if (!reply.WriteInt32(ret)) {
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnUpdateStatsData(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt32(static_cast<int32_t>(UpdateStatsData()))) {
        NETMGR_LOG_E("WriteInt32 failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t NetStatsServiceStub::OnResetFactory(MessageParcel &data, MessageParcel &reply)
{
    if (!reply.WriteInt32(static_cast<int32_t>(ResetFactory()))) {
        NETMGR_LOG_E("WriteInt32 failed");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
