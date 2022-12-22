/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "dns_getaddrinfo.h"

#include <netdb.h>

#include "netnative_log_wrapper.h"
#include "securec.h"
#include "fwmark_client.h"

namespace OHOS {
namespace nmd {
static constexpr int32_t CANNO_LEN = 256;
static constexpr uint32_t LOCAL_ADDR = 0x7f000001;
static constexpr uint32_t ERROR_ADDR = 0x7f000001;
int32_t DnsGetAddrInfo::GetFamily(int32_t &family, uint16_t netId)
{
    static sockaddr_in lo4 = {0};
    {
        lo4.sin_family = AF_INET;
        lo4.sin_port = PORT_NUM;
        lo4.sin_addr.s_addr = __BYTE_ORDER == __BIG_ENDIAN ? LOCAL_ADDR : ERROR_ADDR;
    }
    static sockaddr_in6 lo6 = {0};
    {
        lo6.sin6_family = AF_INET6;
        lo6.sin6_port = PORT_NUM;
        lo6.sin6_addr = IN6ADDR_LOOPBACK_INIT;
    }
    int testFamilys[ARG_INDEX_2] = {AF_INET, AF_INET6};
    const void *testAddrs[ARG_INDEX_2] = {&lo4, &lo6};
    socklen_t testAddrLens[ARG_INDEX_2] = {sizeof lo4, sizeof lo6};
    for (int32_t i = 0; i < FAMILY_TYPE; i++) {
        if (family == testFamilys[1 - i]) {
            continue;
        }
        int32_t socketFd = socket(testFamilys[i], SOCK_CLOEXEC | SOCK_DGRAM, IPPROTO_UDP);
        if (socketFd >= 0) {
            int32_t ret = connect(socketFd, (sockaddr *)testAddrs[i], testAddrLens[i]);
            close(socketFd);
            if (!ret) {
                continue;
            }
        }
        switch (errno) {
            case EADDRNOTAVAIL:
            case EAFNOSUPPORT:
            case EHOSTUNREACH:
            case ENETDOWN:
            case ENETUNREACH:
                break;
            default:
                return EAI_SYSTEM;
        }
        if (family == testFamilys[i]) {
            return EAI_NONAME;
        }
        family = testFamilys[1 - i];
    }
    return 0;
}

int32_t DnsGetAddrInfo::CheckHints(const addrinfo *hint)
{
    int32_t flags = hint->ai_flags;
    const int32_t mask =
        AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_V4MAPPED | AI_ALL | AI_ADDRCONFIG | AI_NUMERICSERV;
    if ((flags & mask) != flags) {
        NETNATIVE_LOGE("flags in hints is Invalid, flags: [%{public}d]", flags);
        return EAI_BADFLAGS;
    }
    int32_t family = hint->ai_family;
    switch (family) {
        case AF_INET:
        case AF_INET6:
        case AF_UNSPEC:
            break;
        default:
            NETNATIVE_LOGE("family not supported, family : [%{public}d]", family);
            return EAI_FAMILY;
    }
    return DNS_ERR_NONE;
}

void DnsGetAddrInfo::ParseAddr(int32_t nAddrs, int32_t nServs, ServData (&ports)[MAXSERVS], AddrData (&addrs)[MAXADDRS],
                               char *outCanon, AddrInfoBuf *out)
{
    int16_t k = 0;
    for (int32_t i = 0; i < nAddrs; i++) {
        for (int32_t j = 0; j < nServs; j++, k++) {
            out[k].slot = k;
            out[k].ai =
                (addrinfo){.ai_family = addrs[i].family,
                           .ai_socktype = ports[j].sockType,
                           .ai_protocol = ports[j].proto,
                           .ai_addrlen = addrs[i].family == AF_INET ? static_cast<socklen_t>(sizeof(sockaddr_in))
                                                                    : static_cast<socklen_t>(sizeof(sockaddr_in6)),
                           .ai_addr = reinterpret_cast<sockaddr *>(&out[k].sa),
                           .ai_canonname = outCanon};
            if (k) {
                out[k - 1].ai.ai_next = &out[k].ai;
            }
            switch (addrs[i].family) {
                case AF_INET:
                    out[k].sa.sin.sin_family = AF_INET;
                    out[k].sa.sin.sin_port = htons(ports[j].port);
                    (void)memcpy_s(&out[k].sa.sin.sin_addr, ADDR_A4_LEN, &addrs[i].addr, ADDR_A4_LEN);
                    break;
                case AF_INET6:
                    out[k].sa.sin6.sin6_family = AF_INET6;
                    out[k].sa.sin6.sin6_port = htons(ports[j].port);
                    out[k].sa.sin6.sin6_scope_id = addrs[i].scopeid;
                    (void)memcpy_s(&out[k].sa.sin6.sin6_addr, ADDR_A6_LEN, &addrs[i].addr, ADDR_A6_LEN);
                    break;
                default:
                    break;
            }
        }
    }
}

int32_t DnsGetAddrInfo::GetAddrInfo(const std::string host, const std::string serv, const addrinfo *hint,
                                    uint16_t netId, addrinfo **res)
{
    if (host.empty() && serv.empty()) {
        return EAI_NONAME;
    }
    int32_t family = AF_UNSPEC;
    int32_t flags = 0;
    int32_t proto = 0;
    int32_t socktype = 0;
    if (hint->ai_family > 0) {
        int32_t error = CheckHints(hint);
        if (error < DNS_ERR_NONE) {
            return error;
        }
        family = hint->ai_family;
        flags = hint->ai_flags;
        proto = hint->ai_protocol;
        socktype = hint->ai_socktype;
    } else {
        family = AF_INET;
    }

    if (flags & AI_ADDRCONFIG) {
        int32_t error = GetFamily(family, netId);
        if (error < DNS_ERR_NONE) {
            return error;
        }
    }
    ServData servBuf[MAXSERVS] = {{0}};
    int32_t nServs = DnsLookUpName().LookUpServer(servBuf, serv, proto, socktype, flags);
    if (nServs < 0) {
        NETNATIVE_LOGE("faild to LookupServ %{public}d", nServs);
        return nServs;
    }
    AddrData addrs[MAXADDRS] = {{0}};
    char canon[CANNO_LEN] = {0};
    int32_t nAddrs = DnsLookUpName().LookUpName(addrs, canon, host, family, flags, netId);
    if (nAddrs < 0) {
        NETNATIVE_LOGE("faild to LookupName %{public}d", nAddrs);
        return nAddrs;
    }
    int32_t nais = nServs * nAddrs;
    int32_t canonLen = strlen(canon);
    AddrInfoBuf *out = static_cast<AddrInfoBuf *>(calloc(1, nais * sizeof(*out) + canonLen + 1));
    if (!out) {
        return EAI_MEMORY;
    }
    char *outCanon = nullptr;
    if (canonLen) {
        outCanon = reinterpret_cast<char *>(&out[nais]);
        if (memcpy_s(outCanon, canonLen + 1, canon, canonLen + 1) != 0) {
            return EAI_AGAIN;
        }
    }
    ParseAddr(nAddrs, nServs, servBuf, addrs, outCanon, out);

    out[0].ref = nais;
    *res = &out->ai;
    return 0;
}
} // namespace nmd
} // namespace OHOS
