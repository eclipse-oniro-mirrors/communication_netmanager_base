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

#include "net_tether_service_proxy.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherServiceProxy::NetTetherServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetTetherService>(impl) {}

NetTetherServiceProxy::~NetTetherServiceProxy() {}

int32_t NetTetherServiceProxy::TetherByIface(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return TETHERING_PARAM_ERR;
    }
    NETMGR_LOG_D("proxy iface[%{public}s]", iface.c_str());
    if (!data.WriteString(iface)) {
        return TETHERING_PARAM_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_TETHER_BY_IFACE, data, reply, option);
    if (retCode != TETHERING_NO_ERR) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return TETHERING_IPC_ERR;
    }
    return reply.ReadInt32();
}

int32_t NetTetherServiceProxy::UntetherByIface(const std::string &iface)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return TETHERING_PARAM_ERR;
    }
    NETMGR_LOG_D("proxy iface[%{public}s]", iface.c_str());
    if (!data.WriteString(iface)) {
        return TETHERING_PARAM_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_UNTETHER_BY_IFACE, data, reply, option);
    if (retCode != TETHERING_NO_ERR) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return TETHERING_IPC_ERR;
    }
    return reply.ReadInt32();
}

int32_t NetTetherServiceProxy::TetherByType(TetheringType type)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return TETHERING_PARAM_ERR;
    }
    NETMGR_LOG_D("proxy type[%{public}d]", static_cast<int32_t>(type));
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        return TETHERING_PARAM_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_TETHER_BY_TYPE, data, reply, option);
    if (retCode != TETHERING_NO_ERR) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return TETHERING_IPC_ERR;
    }
    return reply.ReadInt32();
}

int32_t NetTetherServiceProxy::UntetherByType(TetheringType type)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return TETHERING_PARAM_ERR;
    }
    NETMGR_LOG_D("proxy type[%{public}d]", static_cast<int32_t>(type));
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        return TETHERING_PARAM_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t retCode = remote->SendRequest(CMD_UNTETHER_BY_TYPE, data, reply, option);
    if (retCode != TETHERING_NO_ERR) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return TETHERING_IPC_ERR;
    }
    return reply.ReadInt32();
}

int32_t NetTetherServiceProxy::RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return TETHERING_PARAM_ERR;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return TETHERING_PARAM_ERR;
    }
    data.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return TETHERING_REMOTE_NULLPTR_ERR;
    }
    MessageOption option;
    MessageParcel reply;
    int32_t retCode = remote->SendRequest(CMD_REGISTER_TETHERING_EVENT_CALLBACK, data, reply, option);
    if (retCode != TETHERING_NO_ERR) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", retCode);
        return TETHERING_IPC_ERR;
    }
    return reply.ReadInt32();
}

bool NetTetherServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetTetherServiceProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS