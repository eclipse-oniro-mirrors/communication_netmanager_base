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

#ifndef NET_STATS_CONSTANTS_H
#define NET_STATS_CONSTANTS_H

namespace OHOS {
namespace NetManagerStandard {
static constexpr int16_t LIMIT_STATS_CALLBACK_NUM = 200;
enum class NetStatsResultCode {
    ERR_NONE = 0,
    ERR_INTERNAL_ERROR = (-1),
    ERR_INVALID_PARAMETER = (-2),
    ERR_INVALID_TIME_PERIOD = (-3),
    ERR_READ_BPF_FAIL = (-1001),
    ERR_INVALID_KEY = (-1002),
    ERR_INVALID_IFACE_STATS_MAP = (-1003),
    ERR_INVALID_STATS_VALUE = (-1004),
    ERR_INVALID_STATS_TYPE = (-1005),
    ERR_INVALID_UID_STATS_MAP = (-1006),
    ERR_INVALID_IFACE_NAME_MAP = (-1007),
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_CONSTANTS_H
