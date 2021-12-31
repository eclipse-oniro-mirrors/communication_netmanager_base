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

#ifndef NET_POLICY_TRAFFIC_H
#define NET_POLICY_TRAFFIC_H

#include "singleton.h"
#include "system_ability.h"

#include "i_net_stats_service.h"
#include "net_policy_callback.h"
#include "net_policy_service_stub.h"
#include "net_policy_file.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int16_t NET_POLICY_LEAP_YEAR_ONE = 1;
constexpr int16_t NET_POLICY_LEAP_YEAR_FOUR = 4;
constexpr int16_t NET_POLICY_LEAP_YEAR_ONEHUNDRED = 100;
constexpr int16_t NET_POLICY_LEAP_YEAR_FOURHUNDRED = 400;
constexpr int16_t NET_POLICY_FEBRUARY = 1;
constexpr int32_t NET_POLICY_ONEDAYTIME = 86400;
constexpr int16_t MONTH_TWENTY_EIGHT = 28;
constexpr int16_t MONTH_THIRTY = 30;
constexpr int16_t MONTH_THIRTY_ONE = 31;

class NetPolicyTraffic : public virtual RefBase {
public:
    NetPolicyTraffic(sptr<NetPolicyFile> netPolicyFile);
    NetPolicyResultCode AddUidPolicy(uint32_t uid, NetUidPolicy policy);
    NetPolicyResultCode SetUidPolicy(uint32_t uid, NetUidPolicy policy);
    NetPolicyResultCode DeleteUidPolicy(uint32_t uid, NetUidPolicy policy);
    bool IsUidPolicyExist(uint32_t uid);
    NetPolicyResultCode SetNetPolicys(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys,
        const sptr<NetPolicyCallback> &netPolicyCallback);
    NetPolicyResultCode SetCellularPolicys(const std::vector<NetPolicyCellularPolicy> &cellularPolicys,
        const sptr<NetPolicyCallback> &netPolicyCallback);
    NetPolicyResultCode SnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy,
        const sptr<NetPolicyCallback> &netPolicyCallback);
    NetPolicyResultCode SetIdleWhitelist(uint32_t uid, bool isWhiteList);
    NetPolicyResultCode GetIdleWhitelist(std::vector<uint32_t> &uids);
    void ClearIdleWhiteList();

private:
    bool IsPolicyValid(NetUidPolicy policy);
    bool IsNetPolicyTypeValid(NetQuotaPolicyType netType);
    bool IsNetPolicyPeriodDurationValid(const std::string &periodDuration);
    void CheckNetStatsOverLimit(const std::vector<NetPolicyQuotaPolicy> &quotaPolicys,
        const sptr<NetPolicyCallback> netPolicyCallback);
    void CheckNetStatsOverLimit(const std::vector<NetPolicyCellularPolicy> &cellularPolicys,
        const sptr<NetPolicyCallback> netPolicyCallback);
    int32_t GetPeriodEndTime();
    int64_t GetCurrentTime();
    void InitController();
    bool IsQuotaPolicyExist(const NetPolicyQuotaPolicy &quotaPolicy);

private:
    sptr<NetPolicyFile> netPolicyFile_;
    std::vector<uint16_t> monthDay_;
    std::vector<uint32_t> idleWhiteList_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_TRAFFIC_H
