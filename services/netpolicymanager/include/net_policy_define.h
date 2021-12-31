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

#ifndef NET_POLICY_DEFINE_H
#define NET_POLICY_DEFINE_H

#include "net_policy_cellular_policy.h"

namespace OHOS {
namespace NetManagerStandard {
const mode_t CHOWN_RWX_USR_GRP = 0770;
constexpr int16_t CONVERT_LENGTH_TEN  = 10;
constexpr int16_t CONVERT_LENGTH_EIGHTEEN = 18;
constexpr int16_t PERIODDURATION_POS_NUM_ONE  = 1;
constexpr int16_t DAY_ONE  = 1;
constexpr int16_t DAY_THIRTY_ONE = 31;
constexpr int16_t LIMIT_ACTION_ONE = 1;
constexpr int16_t LIMIT_ACTION_THREE = 3;
const std::string POLICY_QUOTA_MONTH_U = "M";
const std::string POLICY_QUOTA_MONTH_L = "m";
const std::string POLICY_FILE_NAME = "/data/system/net_policy.json";
const std::string CONFIG_HOS_VERSION = "hosVersion";
const std::string CONFIG_UID_POLICY = "uidPolicy";
const std::string CONFIG_UID = "uid";
const std::string CONFIG_POLICY = "policy";
const std::string HOS_VERSION = "1.0";
const std::string CONFIG_BACKGROUND_POLICY = "backgroundPolicy";
const std::string CONFIG_BACKGROUND_POLICY_STATUS = "status";
const std::string CONFIG_QUOTA_POLICY = "quotaPolicy";
const std::string CONFIG_QUOTA_POLICY_NETTYPE = "netType";
const std::string CONFIG_QUOTA_POLICY_SUBSCRIBERID = "subscriberId";
const std::string CONFIG_QUOTA_POLICY_PERIODSTARTTIME = "periodStartTime";
const std::string CONFIG_QUOTA_POLICY_PERIODDURATION = "periodDuration";
const std::string CONFIG_QUOTA_POLICY_WARNINGBYTES = "warningBytes";
const std::string CONFIG_QUOTA_POLICY_LIMITBYTES = "limitBytes";
const std::string CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE = "lastLimitSnooze";
const std::string CONFIG_QUOTA_POLICY_METERED = "metered";
const std::string CONFIG_QUOTA_POLICY_SOURCE = "source";
const std::string CONFIG_CELLULAR_POLICY = "cellularPolicy";
const std::string CONFIG_CELLULAR_POLICY_SUBSCRIBERID = "subscriberId";
const std::string CONFIG_CELLULAR_POLICY_PERIODSTARTTIME = "periodStartTime";
const std::string CONFIG_CELLULAR_POLICY_PERIODDURATION = "periodDuration";
const std::string CONFIG_CELLULAR_POLICY_TITLE = "title";
const std::string CONFIG_CELLULAR_POLICY_SUMMARY = "summary";
const std::string CONFIG_CELLULAR_POLICY_LIMITBYTES = "limitBytes";
const std::string CONFIG_CELLULAR_POLICY_LIMITACTION = "limitAction";
const std::string CONFIG_CELLULAR_POLICY_USEDBYTES = "usedBytes";
const std::string CONFIG_CELLULAR_POLICY_USEDTIMEDURATION = "usedTimeDuration";
const std::string CONFIG_CELLULAR_POLICY_POSSESSOR = "possessor";
const std::string BACKGROUND_POLICY_ALLOW = "allow";
const std::string BACKGROUND_POLICY_REJECT = "reject";

struct UidPolicy {
    std::string uid;
    std::string policy;
};

struct NetPolicyQuota {
    std::string netType;
    std::string subscriberId;
    std::string periodStartTime;
    std::string periodDuration;
    std::string warningBytes;
    std::string limitBytes;
    std::string lastLimitSnooze;
    std::string metered;
    std::string source;
};

struct NetPolicyCellular {
    std::string subscriberId;
    std::string periodStartTime;
    std::string periodDuration;
    std::string title;
    std::string summary;
    std::string limitBytes;
    std::string limitAction;
    std::string usedBytes;
    std::string usedTimeDuration;
    std::string possessor;
};

struct NetPolicy {
    std::string hosVersion;
    std::vector<UidPolicy> uidPolicys;
    std::string backgroundPolicyStatus_;
    std::vector<NetPolicyQuota> netQuotaPolicys;
    std::vector<NetPolicyCellular> netCellularPolicys;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_DEFINE_H