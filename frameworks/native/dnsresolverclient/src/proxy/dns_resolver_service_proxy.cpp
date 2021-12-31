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

#include "dns_resolver_service_proxy.h"
#include "ipc_types.h"
#include "net_mgr_log_wrapper.h"
#include "dns_resolver_constants.h"

namespace OHOS {
namespace NetManagerStandard {
DnsResolverServiceProxy::DnsResolverServiceProxy(const sptr<IRemoteObject> &impl)
    :IRemoteProxy<IDnsResolverService>(impl)
{
}

DnsResolverServiceProxy::~DnsResolverServiceProxy() {}

bool DnsResolverServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(DnsResolverServiceProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}

int32_t DnsResolverServiceProxy::GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo)
{
    MessageParcel data;
    if (hostName.empty()) {
        return NETMANAGER_ERR_STRING_EMPTY;
    }
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(hostName)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_ADDR_BY_NAME, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t vsize = 0;
    if (!reply.ReadInt32(vsize)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    for (int32_t i = 0; i < vsize; ++i) {
        sptr<INetAddr> addr = INetAddr::Unmarshalling(reply);
        addrInfo.push_back(*addr);
    }
    return reply.ReadInt32();
}

int32_t DnsResolverServiceProxy::GetAddrInfo(const std::string &hostName, const std::string &server,
    const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo)
{
    MessageParcel data;
    if (hostName.empty()) {
        return NETMANAGER_ERR_STRING_EMPTY;
    }
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteString(hostName)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteString(server)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    if (!hints->Marshalling(data)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_ADDR_INFO, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t vsize;
    if (!reply.ReadInt32(vsize)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    for (int32_t i = 0; i < vsize; ++i) {
        auto d = DnsAddrInfo::Unmarshalling(reply);
        if (d != nullptr) {
            dnsAddrInfo.push_back(d);
        } else {
            return NETMANAGER_ERR_LOCAL_PTR_NULL;
        }
    }
    return reply.ReadInt32();
}

int32_t DnsResolverServiceProxy::CreateNetworkCache(uint16_t netId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint16(netId)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_CRT_NETWORK_CACHE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DnsResolverServiceProxy::DestoryNetworkCache(uint16_t netId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint16(netId)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_DEL_NETWORK_CACHE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DnsResolverServiceProxy::FlushNetworkCache(uint16_t netId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint16(netId)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_FLS_NETWORK_CACHE, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DnsResolverServiceProxy::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint16(netId)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteUint16(baseTimeoutMsec)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    if (!data.WriteUint8(retryCount)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    int32_t vsize = servers.size();
    if (!data.WriteInt32(vsize)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    for (auto it = servers.begin(); it != servers.end(); ++it) {
        if (!data.WriteString(*it)) {
            return NETMANAGER_ERR_WRITE_DATA_FAIL;
        }
    }
    vsize = domains.size();
    if (!data.WriteInt32(vsize)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    for (auto it = domains.begin(); it != domains.end(); ++it) {
        if (!data.WriteString(*it)) {
            return NETMANAGER_ERR_WRITE_DATA_FAIL;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_SET_RESOLVER_CONFIG, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DnsResolverServiceProxy::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    if (!data.WriteUint16(netId)) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CMD_GET_RESOLVER_INFO, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("proxy SendRequest failed, error code: [%{public}d]", ret);
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    int32_t size = 0;
    if (!reply.ReadInt32(size)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    std::string s;
    for (int32_t i = 0; i < size; ++i) {
        std::string().swap(s);
        if (!reply.ReadString(s)) {
            return NETMANAGER_ERR_READ_REPLY_FAIL;
        }
        servers.push_back(s);
    }
    if (!reply.ReadInt32(size)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    for (int32_t i = 0; i < size; ++i) {
        std::string().swap(s);
        if (!reply.ReadString(s)) {
            return NETMANAGER_ERR_READ_REPLY_FAIL;
        }
        domains.push_back(s);
    }
    if (!reply.ReadUint16(baseTimeoutMsec)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    if (!reply.ReadUint8(retryCount)) {
        return NETMANAGER_ERR_READ_REPLY_FAIL;
    }
    return reply.ReadInt32();
}
} // namespace NetManagerStandard
} // namespace OHOS