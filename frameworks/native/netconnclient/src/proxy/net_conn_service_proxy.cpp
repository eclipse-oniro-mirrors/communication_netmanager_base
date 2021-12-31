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
#include "net_conn_service_proxy.h"

#include <cinttypes>

#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnServiceProxy::NetConnServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetConnService>(impl)
{}

NetConnServiceProxy::~NetConnServiceProxy() {}

int32_t NetConnServiceProxy::SystemReady()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SYSTEM_READY, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
    }
    return error;
}

int32_t NetConnServiceProxy::RegisterNetSupplier(
    uint32_t netType, const std::string &ident, uint64_t netCapabilities, uint32_t &supplierId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }

    NETMGR_LOG_D("proxy netType[%{public}d], ident[%{public}s], netCapabilities[%{public}" PRId64 "]",
        netType, ident.c_str(), netCapabilities);
    if (!data.WriteUint32(netType)) {
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    if (!data.WriteString(ident)) {
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    if (!data.WriteUint64(netCapabilities)) {
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    int32_t error = remote->SendRequest(CMD_NM_REG_NET_SUPPLIER, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        if (!reply.ReadUint32(supplierId)) {
            return IPC_PROXY_ERR;
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::UnregisterNetSupplier(uint32_t supplierId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    NETMGR_LOG_D("proxy supplierId[%{public}d]", supplierId);
    if (!data.WriteUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_UNREG_NETWORK, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::RegisterNetSupplierCallback(uint32_t supplierId,
    const sptr<INetSupplierCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteUint32(supplierId);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK, dataParcel, replyParcel, option);
    NETMGR_LOG_I("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NM_REGISTER_NET_CONN_CALLBACK, dataParcel, replyParcel, option);
    NETMGR_LOG_D("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(
        CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER, dataParcel, replyParcel, option);
    NETMGR_LOG_D("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NM_UNREGISTER_NET_CONN_CALLBACK, dataParcel, replyParcel, option);
    NETMGR_LOG_D("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(
        CMD_NM_UNREGISTER_NET_CONN_CALLBACK_BY_SPECIFIER, dataParcel, replyParcel, option);
    NETMGR_LOG_D("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    NETMGR_LOG_I("Test NetConnServiceProxy::UpdateNetStateForTest(), begin");
    if (netSpecifier == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    netSpecifier->Marshalling(dataParcel);

    if (!dataParcel.WriteInt32(netState)) {
        return NET_CONN_ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NM_UPDATE_NET_STATE_FOR_TEST, dataParcel, replyParcel, option);
    NETMGR_LOG_I("NetConnServiceProxy::UpdateNetStateForTest(), SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetSupplierInfo(uint32_t supplierId,
    const sptr<NetSupplierInfo> &netSupplierInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    NETMGR_LOG_D("proxy supplierId[%{public}d]", supplierId);
    if (!data.WriteUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }
    NETMGR_LOG_D("proxy supplierId[%{public}d] Marshalling success", supplierId);
    if (!netSupplierInfo->Marshalling(data)) {
        NETMGR_LOG_E("proxy Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }
    NETMGR_LOG_D("proxy Marshalling success");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SET_NET_SUPPLIER_INFO, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    NETMGR_LOG_D("proxy supplierId[%{public}d], netCapabilities[%{public}" PRId64 "]", supplierId,
        netCapabilities);
    if (!data.WriteUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteUint64(netCapabilities)) {
        return ERR_FLATTEN_OBJECT;
    }
    NETMGR_LOG_D("proxy Marshalling success");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SET_NET_CAPABILTITES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteUint32(supplierId)) {
        return IPC_PROXY_ERR;
    }

    if (!netLinkInfo->Marshalling(data)) {
        NETMGR_LOG_E("proxy Marshalling failed");
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SET_NET_LINK_INFO, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::RegisterNetDetectionCallback(
    int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    if (!dataParcel.WriteInt32(netId)) {
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }
    MessageParcel replyParcel;
    MessageOption option;
    int32_t error = remote->SendRequest(
        CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK, dataParcel, replyParcel, option);
    if (error != NET_CONN_SUCCESS) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UnRegisterNetDetectionCallback(
    int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    if (!dataParcel.WriteInt32(netId)) {
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }
    MessageParcel replyParcel;
    MessageOption option;
    int32_t error = remote->SendRequest(
        CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK, dataParcel, replyParcel, option);
    if (error != NET_CONN_SUCCESS) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::NetDetection(int32_t netId)
{
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    if (!dataParcel.WriteInt32(netId)) {
        return NET_CONN_ERR_INVALID_PARAMETER;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }
    MessageParcel replyParcel;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_NET_DETECTION, dataParcel, replyParcel, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::GetIfaceNameByType(uint32_t netType, const std::string &ident, std::string &ifaceName)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteUint32(netType)) {
        return IPC_PROXY_ERR;
    }

    if (!data.WriteString(ident)) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_IFACENAME_BY_TYPE, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        if (!reply.ReadString(ifaceName)) {
            return IPC_PROXY_ERR;
        }
    }
    return ret;
}

bool NetConnServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetConnServiceProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}

int32_t NetConnServiceProxy::ActivateNetwork(const sptr<NetSpecifier>& netSpecifier,
    const sptr<INetConnCallback>& callback, uint32_t& reqId)
{
    if (netSpecifier == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier is nullptr");
        return ERR_NULL_OBJECT;
    }

    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is nullptr");
        return ERR_NULL_OBJECT;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());
    MessageOption option;
    MessageParcel replyParcel;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    if (!netSpecifier->Marshalling(dataParcel)) {
        NETMGR_LOG_E("proxy Marshalling failed");
        return IPC_PROXY_ERR;
    }
    int32_t retCode = remote->SendRequest(CMD_NM_ACTIVATENETWORK, dataParcel,
        replyParcel, option);
    NETMGR_LOG_D("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != ERR_NONE) {
        return retCode;
    }
    int32_t ret = 0;
    if (!replyParcel.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        if (!replyParcel.ReadUint32(reqId)) {
            return IPC_PROXY_ERR;
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::DeactivateNetwork(uint32_t& reqId)
{
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }

    if (!dataParcel.WriteUint32(reqId)) {
        return false;
    }

    MessageOption option;
    MessageParcel replyParcel;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    uint32_t errcode = remote->SendRequest(CMD_NM_DEACTIVATENETWORK, dataParcel,
                                           replyParcel, option);
    NETMGR_LOG_D("SendRequest errcode:[%{public}d]", errcode);
    if (errcode != ERR_NONE) {
        return errcode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::GetDefaultNet(int32_t& netId)
{
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }

    if (!dataParcel.WriteInt32(netId)) {
        return IPC_PROXY_ERR;
    }

    MessageOption option;
    MessageParcel replyParcel;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    int32_t errCode = remote->SendRequest(CMD_NM_GETDEFAULTNETWORK, dataParcel,
                                          replyParcel, option);
    NETMGR_LOG_D("SendRequest errcode:[%{public}d]", errCode);
    if (errCode != ERR_NONE) {
        return errCode;
    }
    return replyParcel.ReadInt32();
}


int32_t NetConnServiceProxy::HasDefaultNet(bool &flag)
{
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }

    MessageOption option;
    MessageParcel replyParcel;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t retCode = remote->SendRequest(CMD_NM_HASDEFAULTNET, dataParcel,
        replyParcel, option);
    NETMGR_LOG_D("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != ERR_NONE) {
        return retCode;
    }

    int32_t ret = 0;
    if (!replyParcel.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        if (!replyParcel.ReadBool(flag)) {
            return IPC_PROXY_ERR;
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::GetSpecificNet(uint32_t type, std::list<int32_t> &netIdList)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteUint32(type)) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_SPECIFIC_NET, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        int32_t size = 0;
        if (!reply.ReadInt32(size)) {
            return IPC_PROXY_ERR;
        }
        for (int32_t i = 0; i < size; ++i) {
            uint32_t value ;
            if (!reply.ReadUint32(value)) {
                return IPC_PROXY_ERR;
            }
            netIdList.push_back(value);
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::GetAllNets(std::list<int32_t> &netIdList)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_ALL_NETS, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        int32_t size;
        if (!reply.ReadInt32(size)) {
            return IPC_PROXY_ERR;
        }
        for (int32_t i = 0; i < size; ++i) {
            uint32_t value ;
            if (!reply.ReadUint32(value)) {
                return IPC_PROXY_ERR;
            }
            netIdList.push_back(value);
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::GetSpecificUidNet(int32_t uid, int32_t &netId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteInt32(uid)) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_SPECIFIC_UID_NET, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        if (!reply.ReadInt32(netId)) {
            return IPC_PROXY_ERR;
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::GetConnectionProperties(int32_t netId, NetLinkInfo &info)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteInt32(netId)) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_CONNECTION_PROPERTIES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        sptr<NetLinkInfo> netLinkInfo_ptr = NetLinkInfo::Unmarshalling(reply);
        info = *netLinkInfo_ptr;
    }
    return ret;
}

int32_t NetConnServiceProxy::GetNetCapabilities(int32_t netId, uint64_t &cap)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteInt32(netId)) {
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_NET_CAPABILITIES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        if (!reply.ReadUint64(cap)) {
            return IPC_PROXY_ERR;
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::GetAddressesByName(const std::string &host, int32_t netId, std::list<INetAddr> &addrList)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteString(host)) {
        return IPC_PROXY_ERR;
    }
    if (!data.WriteInt32(netId)) {
        return IPC_PROXY_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_ADDRESSES_BY_NAME, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }

    if (ret == ERR_NONE) {
        int32_t size;
        if (!reply.ReadInt32(size)) {
            return IPC_PROXY_ERR;
        }
        for (int32_t i = 0; i < size; ++i) {
            sptr<INetAddr> netaddr_ptr = INetAddr::Unmarshalling(reply);
            addrList.push_back(*netaddr_ptr);
        }
    }
    return ret;
}

int32_t NetConnServiceProxy::GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteString(host)) {
        return IPC_PROXY_ERR;
    }
    if (!data.WriteInt32(netId)) {
        return IPC_PROXY_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_GET_ADDRESS_BY_NAME, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    if (ret == ERR_NONE) {
        sptr<INetAddr> netaddr_ptr = INetAddr::Unmarshalling(reply);
        addr = *netaddr_ptr;
    }
    return ret;
}

int32_t NetConnServiceProxy::BindSocket(int32_t socket_fd, int32_t netId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteInt32(socket_fd)) {
        return IPC_PROXY_ERR;
    }
    if (!data.WriteInt32(netId)) {
        return IPC_PROXY_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_BIND_SOCKET, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t NetConnServiceProxy::SetAirplaneMode(bool state)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteBool(state)) {
        return IPC_PROXY_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(CMD_NM_SET_AIRPLANE_MODE, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return IPC_PROXY_ERR;
    }
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS
