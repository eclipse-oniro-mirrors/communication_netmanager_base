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

#include "net_tether_service_stub.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherServiceStub::NetTetherServiceStub()
{
    memberFuncMap_[CMD_TETHER_BY_IFACE] = &NetTetherServiceStub::OnTetherByIface;
    memberFuncMap_[CMD_UNTETHER_BY_IFACE] = &NetTetherServiceStub::OnUntetherByIface;
    memberFuncMap_[CMD_TETHER_BY_TYPE] = &NetTetherServiceStub::OnTetherByType;
    memberFuncMap_[CMD_UNTETHER_BY_TYPE] = &NetTetherServiceStub::OnUntetherByType;
    memberFuncMap_[CMD_REGISTER_TETHERING_EVENT_CALLBACK] = &NetTetherServiceStub::OnRegisterTetheringEventCallback;
}

NetTetherServiceStub::~NetTetherServiceStub() {}

int32_t NetTetherServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = NetTetherServiceStub::GetDescriptor();
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

int32_t NetTetherServiceStub::OnTetherByIface(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub processing OnTetherByIface");
    std::string iface;
    if (!data.ReadString(iface)) {
        return TETHERING_PARAM_ERR;
    }
    int32_t ret = TetherByIface(iface);
    if (!reply.WriteInt32(ret)) {
        return TETHERING_PARAM_ERR;
    }
    return TETHERING_NO_ERR;
}

int32_t NetTetherServiceStub::OnUntetherByIface(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub processing OnUntetherByIface");
    std::string iface;
    if (!data.ReadString(iface)) {
        return TETHERING_PARAM_ERR;
    }
    int32_t ret = UntetherByIface(iface);
    if (!reply.WriteInt32(ret)) {
        return TETHERING_PARAM_ERR;
    }
    return TETHERING_NO_ERR;
}

int32_t NetTetherServiceStub::OnTetherByType(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub processing OnTetherByType");
    int32_t type = -1;
    if (!data.ReadInt32(type)) {
        return TETHERING_PARAM_ERR;
    }
    int32_t ret = TetherByType(static_cast<TetheringType>(type));
    if (!reply.WriteInt32(ret)) {
        return TETHERING_PARAM_ERR;
    }
    return TETHERING_NO_ERR;
}

int32_t NetTetherServiceStub::OnUntetherByType(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub processing OnUntetherByType");
    int32_t type = -1;
    if (!data.ReadInt32(type)) {
        return TETHERING_PARAM_ERR;
    }
    int32_t ret = UntetherByType(static_cast<TetheringType>(type));
    if (!reply.WriteInt32(ret)) {
        return TETHERING_PARAM_ERR;
    }
    return TETHERING_NO_ERR;
}

int32_t NetTetherServiceStub::OnRegisterTetheringEventCallback(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub processing OnRegisterTetheringEventCallback");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        reply.WriteInt32(TETHERING_PARAM_ERR);
        return TETHERING_PARAM_ERR;
    }
    sptr<INetTetherCallback> callback = iface_cast<INetTetherCallback>(remote);
    int32_t ret = RegisterTetheringEventCallback(callback);
    if (reply.WriteInt32(ret)) {
        return TETHERING_PARAM_ERR;
    }
    return TETHERING_NO_ERR;
}
} // namespace NetManagerStandard
} // namespace OHOS