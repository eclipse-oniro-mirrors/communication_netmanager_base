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
#include "notify_callback_stub.h"

#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetdNative {
NotifyCallbackStub::NotifyCallbackStub()
{
    memberFuncMap_[ON_INTERFACE_ADDRESS_UPDATED] = &NotifyCallbackStub::CmdOnInterfaceAddressUpdated;
    memberFuncMap_[ON_INTERFACE_ADDRESS_REMOVED] = &NotifyCallbackStub::CmdOnInterfaceAddressRemoved;
    memberFuncMap_[ON_INTERFACE_ADDED] = &NotifyCallbackStub::CmdOnInterfaceAdded;
    memberFuncMap_[ON_INTERFACE_REMOVED] = &NotifyCallbackStub::CmdOnInterfaceRemoved;
    memberFuncMap_[ON_INTERFACE_CHANGED] = &NotifyCallbackStub::CmdOnInterfaceChanged;
    memberFuncMap_[ON_INTERFACE_LINK_STATE_CHANGED] = &NotifyCallbackStub::CmdOnInterfaceLinkStateChanged;
    memberFuncMap_[ON_ROUTE_CHANGED] = &NotifyCallbackStub::CmdOnRouteChanged;
    memberFuncMap_[ON_DHCP_SUCCESS] = &NotifyCallbackStub::CmdDhcpSuccess;
}

NotifyCallbackStub::~NotifyCallbackStub() {}

int32_t NotifyCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETNATIVE_LOGI("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NotifyCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETNATIVE_LOGE("Descriptor checked failed");
        return ERR_FLATTEN_OBJECT;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    NETNATIVE_LOGI("Stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NotifyCallbackStub::CmdOnInterfaceAddressUpdated(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";
    std::string str2 = "";

    int32_t result = OnInterfaceAddressUpdated(str1, str2, 0, 0);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceAddressRemoved(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";
    std::string str2 = "";

    int32_t result = OnInterfaceAddressRemoved(str1, str2, 0, 0);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceAdded(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";
    
    int32_t result = OnInterfaceAdded(str1);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}
int32_t NotifyCallbackStub::CmdOnInterfaceRemoved(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";

    int32_t result = OnInterfaceRemoved(str1);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceChanged(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";

    int32_t result = OnInterfaceChanged(str1, true);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnInterfaceLinkStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";

    int32_t result = OnInterfaceLinkStateChanged(str1, true);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NotifyCallbackStub::CmdOnRouteChanged(MessageParcel &data, MessageParcel &reply)
{
    std::string str1 = "";
    std::string str2 = "";
    std::string str3 = "";

    int32_t result = OnRouteChanged(true, str1, str2, str3);
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
} // namespace NetdNative
} // namespace OHOS
