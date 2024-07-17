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
#include "notify_callback_stub.h"

#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetsysNative {
NotifyCallbackStub::NotifyCallbackStub() {}

NotifyCallbackStub::~NotifyCallbackStub() {}

int32_t NotifyCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETNATIVE_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NotifyCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETNATIVE_LOGE("Descriptor checked failed");
        return ERR_FLATTEN_OBJECT;
    }
    switch (code) {
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_INTERFACE_ADDRESS_UPDATED):
            return CmdOnInterfaceAddressUpdated(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_INTERFACE_ADDRESS_REMOVED):
            return CmdOnInterfaceAddressRemoved(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_INTERFACE_ADDED):
            return CmdOnInterfaceAdded(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_INTERFACE_REMOVED):
            return CmdOnInterfaceRemoved(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_INTERFACE_CHANGED):
            return CmdOnInterfaceChanged(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_INTERFACE_LINK_STATE_CHANGED):
            return CmdOnInterfaceLinkStateChanged(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_ROUTE_CHANGED):
            return CmdOnRouteChanged(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_DHCP_SUCCESS):
            return CmdDhcpSuccess(data, reply);
        case static_cast<uint32_t>(NotifyInterfaceCode::ON_BANDWIDTH_REACHED_LIMIT):
            return CmdOnBandwidthReachedLimit(data, reply);
        default:
            NETNATIVE_LOGI("Stub default case, need check");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t NotifyCallbackStub::CmdOnInterfaceAddressUpdated(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::string ifName = data.ReadString();
    int32_t flags = data.ReadInt32();
    int32_t scope = data.ReadInt32();

    int32_t result = OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceAddressRemoved(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::string ifName = data.ReadString();
    int32_t flags = data.ReadInt32();
    int32_t scope = data.ReadInt32();

    int32_t result = OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceAdded(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = data.ReadString();

    int32_t result = OnInterfaceAdded(ifName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}
int32_t NotifyCallbackStub::CmdOnInterfaceRemoved(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = data.ReadString();

    int32_t result = OnInterfaceRemoved(ifName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceChanged(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = data.ReadString();
    bool up = data.ReadBool();

    int32_t result = OnInterfaceChanged(ifName, up);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceLinkStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = data.ReadString();
    bool up = data.ReadBool();

    int32_t result = OnInterfaceLinkStateChanged(ifName, up);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnRouteChanged(MessageParcel &data, MessageParcel &reply)
{
    bool up = data.ReadBool();
    std::string route = data.ReadString();
    std::string gateway = data.ReadString();
    std::string ifName = data.ReadString();

    int32_t result = OnRouteChanged(up, route, gateway, ifName);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdDhcpSuccess(MessageParcel &data, MessageParcel &reply)
{
    NETNATIVE_LOGI("CmdDhcpSuccess");
    static sptr<DhcpResultParcel> dhcpResult = DhcpResultParcel::Unmarshalling(data);
    OnDhcpSuccess(dhcpResult);
    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnBandwidthReachedLimit(MessageParcel &data, MessageParcel &reply)
{
    std::string limitName = data.ReadString();
    std::string iface = data.ReadString();

    int32_t result = OnBandwidthReachedLimit(limitName, iface);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}
} // namespace NetsysNative
} // namespace OHOS
