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

#include "dns_resolver_service.h"

#include <regex>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>

#include "system_ability_definition.h"
#include "netd_controller.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "dns_resolver_constants.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT_DNS = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<DnsResolverService>::GetInstance().get());
constexpr int32_t IPV4_SIZE = 16;
constexpr int32_t IPV6_SIZE = 48;

DnsResolverService::DnsResolverService()
    : SystemAbility(COMM_DNS_MANAGER_SYS_ABILITY_ID, true)
{}

DnsResolverService::~DnsResolverService() {}

void DnsResolverService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    NETMGR_LOG_D("DnsResolverService::OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("DnsResolverService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("DnsResolverService init failed");
        return;
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, NULL);
    NETMGR_LOG_D("DnsResolverService::OnStart end");
}

void DnsResolverService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool DnsResolverService::Init()
{
    if (!REGISTER_LOCAL_RESULT_DNS) {
        NETMGR_LOG_E("DnsResolverService Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<DnsResolverService>::GetInstance().get())) {
            NETMGR_LOG_E("DnsResolverService Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    serviceIface_ = std::make_unique<DnsServiceIface>().release();
    NetManagerCenter::GetInstance().RegisterDnsService(serviceIface_);
    NETMGR_LOG_D("GetDnsServer suc");
    return true;
}

#ifndef SYS_FUNC
static void FreeAddrInfo2(struct addrinfo *aiHead)
{
    struct addrinfo *ai = nullptr, *aiNext = nullptr;
    for (ai = aiHead; ai != nullptr; ai = aiNext) {
        if (ai->ai_addr != nullptr) {
            free(ai->ai_addr);
            ai->ai_addr = nullptr;
        }
        if (ai->ai_canonname != nullptr) {
            free(ai->ai_canonname);
            ai->ai_canonname = nullptr;
        }
        aiNext = ai->ai_next ;
        free(ai);
        if (aiNext == nullptr) {
            return;
        }
    }
}
#endif

static bool IsDomainValid(const std::string &hostName)
{
    if (hostName.empty()) {
        return false;
    }
    static std::regex domainPattern("([0-9A-Za-z\\-_\\.]+)\\.([0-9a-z]+\\.[a-z]{2,3}(\\.[a-z]{2})?)");
    if (regex_match(hostName, domainPattern)) {
        return true;
    }
    return false;
}

static void ProcessIPV4(const struct addrinfo &addr, std::vector<INetAddr> &addrInfo)
{
    char ip4Buf[IPV4_SIZE];
    bzero(ip4Buf, IPV4_SIZE);
    struct sockaddr_in *sockAddr4 = reinterpret_cast<struct sockaddr_in*>(addr.ai_addr);
    inet_ntop(AF_INET, &sockAddr4->sin_addr, ip4Buf, IPV4_SIZE);
    INetAddr iNetAddr;
    iNetAddr.family_ = addr.ai_family;
    iNetAddr.address_ = std::string(ip4Buf);
    addrInfo.push_back(iNetAddr);
}

static void ProcessIPV6(const struct addrinfo &addr, std::vector<INetAddr> &addrInfo)
{
    char ip6Buf[IPV6_SIZE];
    bzero(ip6Buf, IPV6_SIZE);
    struct sockaddr_in6 *sockAddr6 = reinterpret_cast<struct sockaddr_in6*>(addr.ai_addr);
    inet_ntop(AF_INET6, &sockAddr6->sin6_addr, ip6Buf, IPV6_SIZE);
    INetAddr iNetAddr;
    iNetAddr.family_ = addr.ai_family;
    iNetAddr.address_ = std::string(ip6Buf);
    addrInfo.push_back(iNetAddr);
}

inline void InitAddrInfo(struct addrinfo &hints, int32_t family, int32_t flags, int32_t protocol, int32_t sockType)
{
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_flags = flags;
    hints.ai_protocol = protocol;
    hints.ai_socktype = sockType;
}

int32_t DnsResolverService::GetAddressesByName(const std::string &hostName, std::vector<INetAddr> &addrInfo)
{
    return GetAddressesByName(hostName, 0, addrInfo);
}

int32_t DnsResolverService::GetAddressesByName(const std::string &hostName, int32_t netId,
    std::vector<INetAddr> &addrInfo)
{
    NETMGR_LOG_D("Enter GetAddressesByName.");
    if (!IsDomainValid(hostName)) {
        NETMGR_LOG_E("Invalid domain name format");
        return DNS_ERROR;
    }
    struct addrinfo hints;
    InitAddrInfo(hints, AF_INET, AI_PASSIVE, 0, SOCK_DGRAM);
    std::unique_ptr<addrinfo> res;
    std::string server;
    int32_t ret = NetdController::GetInstance().GetAddrInfo(hostName, server, hints, res, static_cast<uint16_t>(netId));
    if (ret != 0) {
        NETMGR_LOG_E("GetAddressesByName Call GetAddrInfo of NetdController ret[%{public}d]", ret);
        return ret;
    }
    struct addrinfo *cur = nullptr;
    struct addrinfo *resAddr = res.release();
    if (resAddr == nullptr) {
        NETMGR_LOG_E("GetAddrInfo of NetdController return resAddr error");
        return DNS_ERROR;
    }
    for (cur = resAddr; cur != nullptr; cur = cur->ai_next) {
        if (cur->ai_family == AF_INET) {
            ProcessIPV4(*cur, addrInfo);
        } else if (cur->ai_family == AF_INET6) {
            ProcessIPV6(*cur, addrInfo);
        }
    }
    if (resAddr != nullptr) {
#ifdef SYS_FUNC
        freeaddrinfo(resAddr);
#else
        FreeAddrInfo2(resAddr);
#endif
    }
    NETMGR_LOG_E("GetAddressesByName addrInfo size [%{public}zd]", addrInfo.size());
    return DNS_SUCCESS;
}

int32_t DnsResolverService::GetAddrInfo(const std::string &hostName, const std::string &server,
    const sptr<DnsAddrInfo> &hints, std::vector<sptr<DnsAddrInfo>> &dnsAddrInfo)
{
    if (!IsDomainValid(hostName)) {
        return DNS_ERROR;
    }
    struct addrinfo hints2;
    InitAddrInfo(hints2, hints->family_, hints->flags_, hints->protocol_, hints->sockType_);
    std::unique_ptr<addrinfo> res;
    uint16_t netId = 0;
    int32_t ret = NetdController::GetInstance().GetAddrInfo(hostName, server, hints2, res, netId);
    if (ret < 0) {
        return ret;
    }
    struct addrinfo *cur = nullptr;
    struct addrinfo *resAddr = res.release();
    if (resAddr == nullptr) {
        return DNS_ERROR;
    }
    char ipbuf[IPV4_SIZE];
    for (cur = resAddr; cur != nullptr; cur = cur->ai_next) {
        sptr<DnsAddrInfo> d = (std::make_unique<DnsAddrInfo>()).release();
        if (d == nullptr) {
            continue;
        }
        d->flags_ = cur->ai_flags;
        d->family_ = cur->ai_family;
        d->sockType_ = cur->ai_socktype;
        d->protocol_ = cur->ai_protocol;
        if (d->family_ == AF_INET) {
            bzero(ipbuf, IPV4_SIZE);
            auto addr = reinterpret_cast<struct sockaddr_in*>(cur->ai_addr);
            inet_ntop(AF_INET, &addr->sin_addr, ipbuf, IPV4_SIZE);
            d->addr_ = std::string(ipbuf);
        }
        dnsAddrInfo.push_back(d);
    }
    return DNS_SUCCESS;
}

int32_t DnsResolverService::CreateNetworkCache(int32_t netId)
{
    NETMGR_LOG_D("DnsResolverService CreateNetworkCache netId[%{public}d]", netId);
    return static_cast<int32_t>(NetdController::GetInstance().CreateNetworkCache(static_cast<uint16_t>(netId)));
}

int32_t DnsResolverService::DestoryNetworkCache(int32_t netId)
{
    NETMGR_LOG_D("DnsResolverService DestoryNetworkCache netId[%{public}d]", netId);
    return static_cast<int32_t>(NetdController::GetInstance().DestoryNetworkCache(static_cast<uint16_t>(netId)));
}

int32_t DnsResolverService::FlushNetworkCache(int32_t netId)
{
    NETMGR_LOG_D("DnsResolverService FlushNetworkCache netId[%{public}d]", netId);
    return static_cast<int32_t>(NetdController::GetInstance().FlushNetworkCache(static_cast<uint16_t>(netId)));
}

int32_t DnsResolverService::SetResolverConfig(int32_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    NETMGR_LOG_D("DnsResolverService SetResolverConfig netId[%{public}d]", netId);
    return static_cast<int32_t>(NetdController::GetInstance().SetResolverConfig(
        static_cast<uint16_t>(netId), baseTimeoutMsec, retryCount, servers, domains));
}

int32_t DnsResolverService::GetResolverInfo(int32_t netId, std::vector<std::string> &servers,
    std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    NETMGR_LOG_D("DnsResolverService GetResolverInfo netId[%{public}d]", netId);
    return static_cast<int32_t>(NetdController::GetInstance().GetResolverInfo(
        static_cast<uint16_t>(netId), servers, domains, baseTimeoutMsec, retryCount));
}
} // namespace NetManagerStandard
} // namespace OHOS