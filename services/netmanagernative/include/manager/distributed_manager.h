/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_DISTRIBUTED_MANAGER_H
#define NET_DISTRIBUTED_MANAGER_H

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <linux/if.h>
#include "uid_range.h"

namespace OHOS {
namespace NetManagerStandard {

class DistributedManager {
public:
    static DistributedManager &GetInstance()
    {
        static DistributedManager instance;
        return instance;
    }

public:
    int32_t CreateDistributedNic(const std::string &virNicAddr, const std::string &ifName);
    int32_t DestroyDistributedNic(const std::string &ifName);
    int32_t CreateDistributedInterface(const std::string &ifName);
    int32_t SetDistributedNicMtu(const std::string &ifName, int32_t mtu);
    int32_t SetDistributedNicAddress(const std::string &ifName, const std::string &tunAddr);
    void SetServerNicInfo(const std::string &iif, const std::string &devIface);
    void CloseDistributedTunFd();
    std::string GetServerIifNic();
    std::string GetServerDevIfaceNic();
    int32_t ConfigVirnicAndVeth(const std::string &virNicAddr, const std::string &virnicName,
        const std::string &virnicVethName);
    void DisableVirnic(const std::string &virnicName);

private:
    DistributedManager() = default;
    ~DistributedManager() = default;
    DistributedManager(const DistributedManager &) = delete;
    DistributedManager &operator=(const DistributedManager &) = delete;

private:
    int32_t SetDistributedNicUp(const std::string &ifName);
    int32_t SetDistributedNicDown(const std::string &ifName);
    int32_t InitIfreq(ifreq &ifr, const std::string &cardName);
    int32_t SetDistributedNicResult(std::atomic_int &fd, unsigned long cmd, ifreq &ifr);
    void CloseDistributedSocket();

private:
    std::atomic_int tunFd_ = 0;
    std::atomic_int net4Sock_ = 0;
    std::string serverIif_;
    std::string serverDevIface_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_DISTRIBUTED_MANAGER_H
