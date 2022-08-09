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
constexpr int16_t PERIODDURATION_POS_NUM_ONE  = 1;
constexpr int16_t DAY_ONE  = 1;
constexpr int16_t DAY_THIRTY_ONE = 31;
constexpr int16_t LIMIT_ACTION_ONE = 1;
constexpr int16_t LIMIT_ACTION_THREE = 3;
constexpr int16_t LIMIT_CALLBACK_NUM = 200;
constexpr const char *POLICY_QUOTA_MONTH_U = "M";
constexpr const char *POLICY_QUOTA_MONTH_L = "m";
constexpr const char *POLICY_FILE_NAME = "/data/system/net_policy.json";
constexpr const char *CONFIG_HOS_VERSION = "hosVersion";
constexpr const char *CONFIG_UID_POLICY = "uidPolicy";
constexpr const char *CONFIG_UID = "uid";
constexpr const char *CONFIG_POLICY = "policy";
constexpr const char *HOS_VERSION = "1.0";
constexpr const char *CONFIG_BACKGROUND_POLICY = "backgroundPolicy";
constexpr const char *CONFIG_BACKGROUND_POLICY_STATUS = "status";
constexpr const char *CONFIG_QUOTA_POLICY = "quotaPolicy";
constexpr const char *CONFIG_QUOTA_POLICY_NETTYPE = "netType";
constexpr const char *CONFIG_QUOTA_POLICY_SUBSCRIBERID = "simId";
constexpr const char *CONFIG_QUOTA_POLICY_PERIODSTARTTIME = "periodStartTime";
constexpr const char *CONFIG_QUOTA_POLICY_PERIODDURATION = "periodDuration";
constexpr const char *CONFIG_QUOTA_POLICY_WARNINGBYTES = "warningBytes";
constexpr const char *CONFIG_QUOTA_POLICY_LIMITBYTES = "limitBytes";
constexpr const char *CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE = "lastLimitSnooze";
constexpr const char *CONFIG_QUOTA_POLICY_METERED = "metered";
constexpr const char *CONFIG_QUOTA_POLICY_SOURCE = "source";
constexpr const char *CONFIG_CELLULAR_POLICY = "cellularPolicy";
constexpr const char *CONFIG_CELLULAR_POLICY_SUBSCRIBERID = "simId";
constexpr const char *CONFIG_CELLULAR_POLICY_PERIODSTARTTIME = "periodStartTime";
constexpr const char *CONFIG_CELLULAR_POLICY_PERIODDURATION = "periodDuration";
constexpr const char *CONFIG_CELLULAR_POLICY_TITLE = "title";
constexpr const char *CONFIG_CELLULAR_POLICY_SUMMARY = "summary";
constexpr const char *CONFIG_CELLULAR_POLICY_LIMITBYTES = "limitBytes";
constexpr const char *CONFIG_CELLULAR_POLICY_LIMITACTION = "limitAction";
constexpr const char *CONFIG_CELLULAR_POLICY_USEDBYTES = "usedBytes";
constexpr const char *CONFIG_CELLULAR_POLICY_USEDTIMEDURATION = "usedTimeDuration";
constexpr const char *CONFIG_CELLULAR_POLICY_POSSESSOR = "possessor";
constexpr const char *BACKGROUND_POLICY_ALLOW = "allow";
constexpr const char *BACKGROUND_POLICY_REJECT = "reject";
constexpr const char *IDENT_PREFIX = "usb0";

struct UidPolicy {
    std::string uid;
    std::string policy;
};

struct NetPolicyQuota {
    std::string netType;
    std::string simId;
    std::string periodStartTime;
    std::string periodDuration;
    std::string warningBytes;
    std::string limitBytes;
    std::string lastLimitSnooze;
    std::string metered;
    std::string source;
};

struct NetPolicyCellular {
    std::string simId;
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
