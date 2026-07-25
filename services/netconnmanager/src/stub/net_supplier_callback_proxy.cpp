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

#include "net_mgr_log_wrapper.h"
#include "net_supplier_callback_proxy.h"
#include "net_manager_constants.h"
namespace OHOS {
namespace NetManagerStandard {
NetSupplierCallbackProxy::NetSupplierCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetSupplierCallback>(impl)
{}

NetSupplierCallbackProxy::~NetSupplierCallbackProxy() {}

static bool WriteRequestNetworkData(MessageParcel &data, const std::string &ident,
    const std::set<NetCap> &netCaps, const NetRequest &netrequest)
{
    if (!data.WriteString(ident)) {
        NETMGR_LOG_E("Write ident failed");
        return false;
    }
    uint32_t size = static_cast<uint32_t>(netCaps.size());
    if (!data.WriteUint32(size)) {
        NETMGR_LOG_E("Write netCaps size failed");
        return false;
    }
    for (auto netCap : netCaps) {
        if (!data.WriteUint32(static_cast<uint32_t>(netCap))) {
            NETMGR_LOG_E("Write netCap failed");
            return false;
        }
    }
    if (!data.WriteUint32(netrequest.registerType)) {
        NETMGR_LOG_E("Write registerType failed");
        return false;
    }
    uint32_t bearTypeSize = static_cast<uint32_t>(netrequest.bearTypes.size());
    if (!data.WriteUint32(bearTypeSize)) {
        NETMGR_LOG_E("Write bearTypes size failed");
        return false;
    }
    for (auto bearType : netrequest.bearTypes) {
        if (!data.WriteUint32(static_cast<uint32_t>(bearType))) {
            NETMGR_LOG_E("Write bearType failed");
            return false;
        }
    }
    if (!data.WriteUint32(netrequest.uid)) {
        NETMGR_LOG_E("Write uid failed");
        return false;
    }
    if (!data.WriteUint32(netrequest.requestId)) {
        NETMGR_LOG_E("Write requestId failed");
        return false;
    }
    if (!data.WriteString(netrequest.ident)) {
        NETMGR_LOG_E("Write ident failed");
        return false;
    }
    return true;
}

int32_t NetSupplierCallbackProxy::RequestNetwork(const std::string &ident, const std::set<NetCap> &netCaps,
                                                 const NetRequest &netrequest)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!WriteRequestNetworkData(data, ident, netCaps, netrequest)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(SupplierInterfaceCode::NET_SUPPLIER_REQUEST_NETWORK), data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

static bool WriteReleaseParcelData(MessageParcel &data, const NetRequest &netRequest)
{
    bool result = data.WriteUint32(netRequest.uid) && data.WriteUint32(netRequest.requestId) &&
                  data.WriteUint32(netRequest.registerType) && data.WriteString(netRequest.ident);
    if (!result) {
        NETMGR_LOG_E("Write uid, requestId, registerType or ident failed");
        return false;
    }
    uint32_t size = static_cast<uint32_t>(netRequest.bearTypes.size());
    if (!data.WriteUint32(size)) {
        NETMGR_LOG_E("Write bearTypes size failed");
        return false;
    }
    for (auto netBearType : netRequest.bearTypes) {
        if (!data.WriteUint32(static_cast<uint32_t>(netBearType))) {
            NETMGR_LOG_E("Write net BearType failed");
            return false;
        }
    }
    size = static_cast<uint32_t>(netRequest.netCaps.size());
    if (!data.WriteUint32(size)) {
        NETMGR_LOG_E("Write net caps size failed");
        return false;
    }
    for (auto netCap : netRequest.netCaps) {
        if (!data.WriteUint32(static_cast<uint32_t>(netCap))) {
            NETMGR_LOG_E("Write net cap failed");
            return false;
        }
    }
    return true;
}

int32_t NetSupplierCallbackProxy::ReleaseNetwork(const NetRequest &netRequest)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!WriteReleaseParcelData(data, netRequest)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(SupplierInterfaceCode::NET_SUPPLIER_RELEASE_NETWORK), data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

bool NetSupplierCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetSupplierCallbackProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
