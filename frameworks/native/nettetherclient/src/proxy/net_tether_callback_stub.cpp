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
#include "net_tether_callback_stub.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherCallbackStub::NetTetherCallbackStub()
{
    memberFuncMap_[NET_TETHER_SUCCESS] = &NetTetherCallbackStub::OnTetherSuccess;
    memberFuncMap_[NET_TETHER_FAILED] = &NetTetherCallbackStub::OnTetherFailed;
}

NetTetherCallbackStub::~NetTetherCallbackStub() {}

int32_t NetTetherCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    NETMGR_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetTetherCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_D("Descriptor checked failed");
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

int32_t NetTetherCallbackStub::OnTetherSuccess(MessageParcel &data, MessageParcel &reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_W("sent raw data is less than 32k");
    }
    int32_t type = -1;
    if (!data.ReadInt32(type)) {
        return ERR_FLATTEN_OBJECT;
    }
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = TetherSuccess(type, iface);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetTetherCallbackStub::OnTetherFailed(MessageParcel &data, MessageParcel &reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_W("sent raw data is less than 32k");
    }
    int32_t type = -1;
    if (!data.ReadInt32(type)) {
        return ERR_FLATTEN_OBJECT;
    }
    std::string iface;
    if (!data.ReadString(iface)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t failCode = -1;
    if (!data.ReadInt32(failCode)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = TetherFailed(type, iface, failCode);
    if (!reply.WriteInt32(result)) {
        NETMGR_LOG_E("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}
}  // namespace NetManagerStandard
}  // namespace OHOS