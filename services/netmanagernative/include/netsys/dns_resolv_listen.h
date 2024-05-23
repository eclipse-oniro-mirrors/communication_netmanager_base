/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef NETSYS_DNSRESOLV_DNS_RESOLV_LISTEN_H
#define NETSYS_DNSRESOLV_DNS_RESOLV_LISTEN_H

#include <cstdint>

#include "event_handler.h"
#include "ffrt.h"

namespace OHOS {
namespace nmd {
class DnsResolvListen {
public:
    DnsResolvListen();
    ~DnsResolvListen();

    void StartListen();

private:
    static void ProcGetConfigCommand(int clientSockFd, uint16_t netId);
    static void ProcSetCacheCommand(int clientSockFd, uint16_t netId);
    static void ProcGetCacheCommand(int clientSockFd, uint16_t netId);
    static void ProcJudgeIpv6Command(int clientSockFd, uint16_t netId);
    static void ProcPostDnsResultCommand(int clientSockFd, uint16_t netId);
    static void ProcGetDefaultNetworkCommand(int clientSockFd, uint16_t netId);
    static void ProcBindSocketCommand(int clientSockFd, uint16_t netId);
    static void ProcCommand(int clientSockFd);
    static int32_t ProcGetKeyForCache(int clientSockFd, char *name);
    static bool ProcPostDnsThreadResult(int clientSockFd, uint32_t &uid, uint32_t &pid);

    int32_t serverSockFd_;
    std::shared_ptr<ffrt::queue> dnsResolvListenFfrtQueue_ = nullptr;
};
} // namespace nmd
} // namespace OHOS
#endif // NETSYS_DNSRESOLV_DNS_RESOLV_LISTEN_H
