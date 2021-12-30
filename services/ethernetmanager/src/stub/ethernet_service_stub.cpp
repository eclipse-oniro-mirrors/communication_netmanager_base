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

#include "ethernet_service_stub.h"
#include "net_mgr_log_wrapper.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
EthernetServiceStub::EthernetServiceStub()
{
    memberFuncMap_[CMD_SET_IF_CFG] = &EthernetServiceStub::OnSetIfaceConfig;
    memberFuncMap_[CMD_GET_IF_CFG] = &EthernetServiceStub::OnGetIfaceConfig;
    memberFuncMap_[CMD_IS_ACTIVATE] = &EthernetServiceStub::OnIsActivate;
    memberFuncMap_[CMD_GET_ACTIVATE_INTERFACE] = &EthernetServiceStub::OnGetActivateInterfaces;
}

EthernetServiceStub::~EthernetServiceStub() {}

int32_t EthernetServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = EthernetServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_E("descriptor checked fail");
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
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

int32_t EthernetServiceStub::OnSetIfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    sptr<InterfaceConfiguration> ic = InterfaceConfiguration::Unmarshalling(data);
    if (ic == nullptr) {
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    int32_t ret = SetIfaceConfig(iface, ic);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetServiceStub::OnGetIfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    sptr<InterfaceConfiguration> ic = GetIfaceConfig(iface);
    if (ic != nullptr) {
        if (!reply.WriteInt32(GET_CFG_SUC)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        if (!ic->Marshalling(reply)) {
            NETMGR_LOG_E("proxy Marshalling failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    } else {
        if (!reply.WriteInt32(0)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetServiceStub::OnIsActivate(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = IsActivate(iface);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t EthernetServiceStub::OnGetActivateInterfaces(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> ifaces = GetActivateInterfaces();
    if (!reply.WriteInt32(ifaces.size())) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    for (auto it = ifaces.begin(); it != ifaces.end(); ++it) {
        if (!reply.WriteString(*it)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS