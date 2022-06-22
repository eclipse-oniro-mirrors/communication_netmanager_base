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

#ifndef NETLINK_DEFINE_H
#define NETLINK_DEFINE_H

#include <string>
#include <vector>

namespace OHOS {
namespace nmd {
namespace NetlinkDefine {
static const int32_t NETLINK_FORMAT_ASCII = 0;
static const int32_t NETLINK_FORMAT_BINARY = 1;
static const int32_t NETLINK_FORMAT_BINARY_UNICAST = 2;

static constexpr int32_t BUFFER_SIZE = 64 * 1024;
static constexpr int32_t DECIMALISM = 10;

static constexpr int32_t INDEX_ZERO = 0;
static constexpr int32_t INDEX_ONE = 1;
static constexpr int32_t INDEX_TWO = 2;
static constexpr int32_t INDEX_THREE = 3;
static constexpr int32_t INDEX_FOUR = 4;
static constexpr int32_t INDEX_FIVE = 5;
static constexpr int32_t INDEX_SIX = 6;
static constexpr int32_t INDEX_SEVEN = 7;
static constexpr int32_t INDEX_EIGHT = 8;
static constexpr int32_t INDEX_NINE = 9;
} // namespace NetlinkUtils

namespace NetlinkResult {
static constexpr int32_t OK = 0;
static constexpr int32_t ERROR = -1;
} // namespace NetlinkResult
} // namespace nmd
} // namespace OHOS

#endif // !NETLINK_DEFINE_H
