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
#include "net_conn_callback_proxy.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnCallbackProxy::NetConnCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetConnCallback>(impl)
{}

NetConnCallbackProxy::~NetConnCallbackProxy() {}

int32_t NetConnCallbackProxy::NetAvailable(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        NETMGR_LOG_E("netHandle is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(netHandle->GetNetId())) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(NET_AVAILABLE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetConnCallbackProxy::NetCapabilitiesChange(
    sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (netHandle == nullptr || netAllCap == nullptr) {
        return ERR_NULL_OBJECT;
    }

    if (!data.WriteInt32(netHandle->GetNetId()) || !data.WriteUint32(netAllCap->linkUpBandwidthKbps_) ||
        !data.WriteUint32(netAllCap->linkDownBandwidthKbps_)) {
        return IPC_PROXY_ERR;
    }
    uint32_t size = static_cast<uint32_t>(netAllCap->netCaps_.size());
    if (!data.WriteUint32(size)) {
        return IPC_PROXY_ERR;
    }
    for (auto netCap : netAllCap->netCaps_) {
        if (!data.WriteUint32(static_cast<uint32_t>(netCap))) {
            return IPC_PROXY_ERR;
        }
    }
    size = static_cast<uint32_t>(netAllCap->bearerTypes_.size());
    if (!data.WriteUint32(size)) {
        return IPC_PROXY_ERR;
    }
    for (auto bearerType : netAllCap->bearerTypes_) {
        if (!data.WriteUint32(static_cast<uint32_t>(bearerType))) {
            return IPC_PROXY_ERR;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(NET_CAPABILITIES_CHANGE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetConnCallbackProxy::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info)
{
    if (netHandle == nullptr || info == nullptr) {
        NETMGR_LOG_E("Input parameter is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(netHandle->GetNetId())) {
        return IPC_PROXY_ERR;
    }

    if (!info->Marshalling(data)) {
        NETMGR_LOG_E("proxy Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(NET_CONNECTION_PROPERTIES_CHANGE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetConnCallbackProxy::NetLost(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        NETMGR_LOG_E("netHandle is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(netHandle->GetNetId())) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(NET_LOST, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetConnCallbackProxy::NetUnavailable()
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
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(NET_UNAVAILABLE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetConnCallbackProxy::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    if (netHandle == nullptr) {
        NETMGR_LOG_E("netHandle is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(netHandle->GetNetId())) {
        return IPC_PROXY_ERR;
    }
    if (!data.WriteBool(blocked)) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(NET_BLOCK_STATUS_CHANGE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

bool NetConnCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetConnCallbackProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
