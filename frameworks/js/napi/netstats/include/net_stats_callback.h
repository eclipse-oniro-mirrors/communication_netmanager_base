/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_NET_STATS_CALLBACK_H
#define NETMANAGER_BASE_NET_STATS_CALLBACK_H

#include <mutex>
#include <condition_variable>

#include "net_stats_callback_stub.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
struct IfaceStatsEvent {
    IfaceStatsEvent(std::string iface) : iface(iface) {}
    std::string iface;
};

struct UidStatsEvent {
    UidStatsEvent(std::string iface, uint32_t uid) : iface(iface), uid(uid) {}
    std::string iface;
    uint32_t uid;
};

class NetStatsCallback : public NetStatsCallbackStub {
public:
    int32_t NetIfaceStatsChanged(const std::string &iface);
    int32_t NetUidStatsChanged(const std::string &iface, uint32_t uid);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_BASE_NET_STATS_CALLBACK_H
