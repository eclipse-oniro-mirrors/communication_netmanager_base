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
#include "net_conn_callback_stub.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnCallbackStub::NetConnCallbackStub()
{
    memberFuncMap_[NET_CONN_STATE_CHANGED] = &NetConnCallbackStub::OnNetConnStateChanged;
    memberFuncMap_[NET_AVAILIABLE] = &NetConnCallbackStub::OnNetAvailable;
    memberFuncMap_[NET_CAPABILITIES_CHANGE] = &NetConnCallbackStub::OnNetCapabilitiesChange;
    memberFuncMap_[NET_CONNECTION_PROPERTIES_CHANGE] = &NetConnCallbackStub::OnNetConnectionPropertiesChange;
    memberFuncMap_[NET_LOST] = &NetConnCallbackStub::OnNetLost;
}

NetConnCallbackStub::~NetConnCallbackStub() {}

int32_t NetConnCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetConnCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_E("Descriptor checked failed");
        return ERR_FLATTEN_OBJECT;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    NETMGR_LOG_D("Stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetConnCallbackStub::OnNetConnStateChanged(MessageParcel &data, MessageParcel &reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_E("Execute ContainFileDescriptors failed");
    }

    sptr<NetConnCallbackInfo> info = NetConnCallbackInfo::Unmarshalling(data);
    int32_t result = NetConnStateChanged(info);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetConnCallbackStub::OnNetAvailable(MessageParcel& data, MessageParcel& reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_W("sent raw data is less than 32k");
    }
    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return IPC_PROXY_ERR;
    }
    int32_t result = NetAvailable(netId);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetConnCallbackStub::OnNetCapabilitiesChange(MessageParcel& data, MessageParcel& reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_W("sent raw data is less than 32k");
    }

    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return IPC_PROXY_ERR;
    }
    uint64_t netCap;
    if (!data.ReadUint64(netCap)) {
        return IPC_PROXY_ERR;
    }
    int32_t result = NetCapabilitiesChange(netId, netCap);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetConnCallbackStub::OnNetConnectionPropertiesChange(MessageParcel& data, MessageParcel& reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_W("sent raw data is less than 32k");
    }

    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return IPC_PROXY_ERR;
    }
    sptr<NetLinkInfo> info = NetLinkInfo::Unmarshalling(data);
    int32_t result = NetConnectionPropertiesChange(netId, info);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetConnCallbackStub::OnNetLost(MessageParcel& data, MessageParcel& reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_W("sent raw data is less than 32k");
    }

    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return IPC_PROXY_ERR;
    }

    int32_t result = NetLost(netId);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}
}  // namespace NetManagerStandard
}  // namespace OHOS
