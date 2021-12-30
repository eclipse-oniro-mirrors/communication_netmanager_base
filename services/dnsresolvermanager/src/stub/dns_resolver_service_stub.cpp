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

#include "dns_resolver_service_stub.h"
#include "net_mgr_log_wrapper.h"
#include "dns_resolver_constants.h"

namespace OHOS {
namespace NetManagerStandard {
DnsResolverServiceStub::DnsResolverServiceStub()
{
    memberFuncMap_[CMD_GET_ADDR_BY_NAME] = &DnsResolverServiceStub::OnGetAddressesByName;
    memberFuncMap_[CMD_GET_ADDR_INFO] = &DnsResolverServiceStub::OnGetAddrInfo;
    memberFuncMap_[CMD_CRT_NETWORK_CACHE] = &DnsResolverServiceStub::OnCreateNetworkCache;
    memberFuncMap_[CMD_DEL_NETWORK_CACHE] = &DnsResolverServiceStub::OnDestoryNetworkCache;
    memberFuncMap_[CMD_FLS_NETWORK_CACHE] = &DnsResolverServiceStub::OnFlushNetworkCache;
    memberFuncMap_[CMD_SET_RESOLVER_CONFIG] = &DnsResolverServiceStub::OnSetResolverConfig;
    memberFuncMap_[CMD_GET_RESOLVER_INFO] = &DnsResolverServiceStub::OnGetResolverInfo;
}

DnsResolverServiceStub::~DnsResolverServiceStub() {}

int32_t DnsResolverServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = DnsResolverServiceStub::GetDescriptor();
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

int32_t DnsResolverServiceStub::OnGetAddressesByName(MessageParcel &data, MessageParcel &reply)
{
    std::string hostName;
    if (!data.ReadString(hostName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    std::vector<INetAddr> addrInfo;
    int32_t ret = GetAddressesByName(hostName, addrInfo);
    if (!reply.WriteInt32(addrInfo.size())) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    for (auto s : addrInfo) {
        if (!s.Marshalling(reply)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DnsResolverServiceStub::OnGetAddrInfo(MessageParcel &data, MessageParcel &reply)
{
    std::string hostname;
    std::string server;
    if (!data.ReadString(hostname)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (!data.ReadString(server)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    sptr<DnsAddrInfo> hints = DnsAddrInfo::Unmarshalling(data);
    std::vector<sptr<DnsAddrInfo>> dnsAddrInfo;
    int32_t ret = GetAddrInfo(hostname, server, hints, dnsAddrInfo);

    int32_t vsize = dnsAddrInfo.size();
    if (!reply.WriteInt32(vsize)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    for (std::vector<sptr<DnsAddrInfo>>::iterator it = dnsAddrInfo.begin(); it != dnsAddrInfo.end(); ++it) {
        (*it)->Marshalling(reply);
    }
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DnsResolverServiceStub::OnCreateNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = 0;
    if (!data.ReadUint16(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = CreateNetworkCache(netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DnsResolverServiceStub::OnDestoryNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = 0;
    if (!data.ReadUint16(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = DestoryNetworkCache(netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DnsResolverServiceStub::OnFlushNetworkCache(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = 0;
    if (!data.ReadUint16(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = FlushNetworkCache(netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DnsResolverServiceStub::OnSetResolverConfig(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = 0;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    std::vector<std::string> servers;
    std::vector<std::string> domains;

    if (!data.ReadUint16(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (!data.ReadUint16(baseTimeoutMsec)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (!data.ReadUint8(retryCount)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t size;
    if (!data.ReadInt32(size)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string s;
    for (int32_t i = 0; i < size; ++i) {
        std::string().swap(s);
        if (!data.ReadString(s)) {
            return NETMANAGER_ERR_READ_DATA_FAIL;
        }
        servers.push_back(s);
    }

    if (!data.ReadInt32(size)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    for (int32_t i = 0; i < size; ++i) {
        std::string().swap(s);
        if (!data.ReadString(s)) {
            return NETMANAGER_ERR_READ_DATA_FAIL;
        }
        domains.push_back(s);
    }
    int32_t ret = SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t DnsResolverServiceStub::OnGetResolverInfo(MessageParcel &data, MessageParcel &reply)
{
    uint16_t netId = 0;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    if (!data.ReadUint16(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = GetResolverInfo(netId, servers, domains, baseTimeoutMsec, retryCount);

    int32_t vsize = servers.size();
    if (!reply.WriteInt32(vsize)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    for (std::vector<std::string>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (!reply.WriteString(*it)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    vsize = domains.size();
    if (!reply.WriteInt32(vsize)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    for (std::vector<std::string>::iterator it = domains.begin(); it != domains.end(); ++it) {
        if (!reply.WriteString(*it)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }

    if (!reply.WriteUint16(baseTimeoutMsec)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (!reply.WriteUint8(retryCount)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS