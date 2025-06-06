/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MULTI_VPN_MANAGER_H
#define MULTI_VPN_MANAGER_H

#include <cstdint>
#include <linux/if.h>
#include <string>
#include <mutex>

namespace OHOS {
namespace NetManagerStandard {
constexpr const char *XFRM_CARD_NAME = "xfrm-vpn";
constexpr const char *PPP_CARD_NAME = "ppp";
constexpr const char *PPP_DEVICE_PATH = "/dev/ppp";

class MultiVpnManager : public std::enable_shared_from_this<MultiVpnManager> {
private:
    MultiVpnManager(const MultiVpnManager &) = delete;
    MultiVpnManager &operator=(const MultiVpnManager &) = delete;

public:
    MultiVpnManager() = default;
    ~MultiVpnManager() = default;
    static MultiVpnManager &GetInstance()
    {
        static std::shared_ptr<MultiVpnManager> instance = std::make_shared<MultiVpnManager>();
        return *instance;
    }

public:
    int32_t CreateVpnInterface(const std::string &ifName);
    int32_t DestroyVpnInterface(const std::string &ifName);
    int32_t SetVpnAddress(const std::string &ifName, const std::string &vpnAddr, int32_t prefix);
    int32_t SetVpnMtu(const std::string &ifName, int32_t mtu);
    int32_t CreatePppFd(const std::string &ifName);
    void SetXfrmPhyIfName(const std::string &phyName);
    void SetVpnRemoteAddress(const std::string &remoteIp);

private:
    int32_t SendVpnInterfaceFdToClient(int32_t clientFd, int32_t tunFd);
    int32_t SetVpnResult(std::atomic_int &fd, unsigned long cmd, ifreq &ifr);
    int32_t InitIfreq(ifreq &ifr, const std::string &ifName);
    int32_t SetVpnDown(const std::string &ifName);
    int32_t SetVpnUp(const std::string &ifName);
    int32_t AddVpnRemoteAddress(const std::string &ifName, std::atomic_int &net4Sock, ifreq &ifr);
    int32_t CreatePppInterface(const std::string &ifName);
    int32_t DestroyPppFd(const std::string &ifName);
    void StartPppSocketListen(const std::string &ifName);
    void StartPppInterfaceFdListen(const std::string &ifName);

private:
    std::atomic_bool pppListeningFlag_ = false;
    std::unordered_map<std::string, std::atomic_int> pppFdMap_;
    std::string phyName_;
    std::string remoteIpv4Addr_;
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MULTI_VPN_MANAGER_H
