/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "net_stats_history.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t NetStatsHistory::GetHistory(std::vector<NetStatsInfo> &recv, uint64_t start, uint64_t end)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ReadStatsData(recv, start, end);
}

int32_t NetStatsHistory::GetHistory(std::vector<NetStatsInfo> &recv, uint32_t uid, uint64_t start, uint64_t end)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ReadStatsData(recv, uid, start, end);
}

int32_t NetStatsHistory::GetHistory(std::vector<NetStatsInfo> &recv, const std::string &iface, uint64_t start,
                                    uint64_t end)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ReadStatsData(recv, iface, start, end);
}

int32_t NetStatsHistory::GetHistory(std::vector<NetStatsInfo> &recv, const std::string &iface, uint32_t uid,
                                    uint64_t start, uint64_t end)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ReadStatsData(recv, iface, uid, start, end);
}

int32_t GetHistoryByIdent(std::vector<NetStatsInfo> &recv, const std::string &ident, uint64_t start, uint64_t end)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ReadStatsDataByIdent(recv, ident, start, end);
}

int32_t GetHistory(std::vector<NetStatsInfo> &recv, uint32_t uid, const std::string &ident, uint64_t start,
                   uint64_t end)
{
    auto handler = std::make_unique<NetStatsDataHandler>();
    return handler->ReadStatsData(recv, uid, ident, start, end);
}
} // namespace NetManagerStandard
} // namespace OHOS