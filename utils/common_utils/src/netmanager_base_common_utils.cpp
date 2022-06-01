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

#include "netmanager_base_common_utils.h"
#include "securec.h"

#include <algorithm>
#include <arpa/inet.h>

namespace OHOS::NetManagerStandard::CommonUtils {
constexpr int32_t BIT32 = 32;
constexpr int32_t BIT24 = 24;
constexpr int32_t BIT16 = 16;
constexpr int32_t BIT8 = 8;
constexpr int32_t INET_PTION_SUC = 1;
std::vector<std::string> Split(const std::string &str, const std::string &sep)
{
    std::string s = str;
    std::vector<std::string> res;
    while (!s.empty()) {
        size_t pos = s.find(sep);
        if (pos == std::string::npos) {
            res.emplace_back(s);
            break;
        }
        res.emplace_back(s.substr(0, pos));
        s = s.substr(pos + sep.size());
    }
    return res;
}

std::string Strip(const std::string &str, char ch)
{
    int64_t i = 0;
    while (i < str.size() && str[i] == ch) {
        ++i;
    }
    int64_t j = static_cast<int64_t>(str.size()) - 1;
    while (j > 0 && str[j] == ch) {
        --j;
    }
    if (i >= 0 && i < str.size() && j >= 0 && j < str.size() && j - i + 1 > 0) {
        return str.substr(i, j - i + 1);
    }
    return "";
}

std::string ToLower(const std::string &s)
{
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), tolower);
    return res;
}

bool IsValidIPV4(const std::string &ip)
{
    if (ip.empty()) {
        return false;
    }
    struct in_addr s;
    int32_t result = inet_pton(AF_INET, ip.c_str(), reinterpret_cast<void*>(&s));
    if (result == INET_PTION_SUC) {
        return true;
    }
    return false;
}

bool IsValidIPV6(const std::string &ip)
{
    if (ip.empty()) {
        return false;
    }
    struct in6_addr s;
    int32_t result = inet_pton(AF_INET6, ip.c_str(), reinterpret_cast<void*>(&s));
    if (result == INET_PTION_SUC) {
        return true;
    }
    return false;
}

int8_t GetAddrFamily(const std::string &ip)
{
    if (IsValidIPV4(ip)) {
        return AF_INET;
    }
    if (IsValidIPV6(ip)) {
        return AF_INET6;
    }
    return 0;
}

int32_t Ipv4PrefixLen(const std::string &ip)
{
    if (ip.empty()) {
        return 0;
    }
    int32_t ret = 0;
    uint32_t ipNum = 0;
    uint8_t c1 = 0;
    uint8_t c2 = 0;
    uint8_t c3 = 0;
    uint8_t c4 = 0;
    int32_t cnt = 0;
    ret = sscanf_s(ip.c_str(), "%hhu.%hhu.%hhu.%hhu", &c1, &c2, &c3, &c4);
    if (ret != sizeof(int32_t)) {
        return 0;
    }
    ipNum = (c1 << BIT24) | (c2 << BIT16) | (c3 << BIT8) | c4;
    if (ipNum == 0xFFFFFFFF) {
        return BIT32;
    }
    if (ipNum == 0xFFFFFF00) {
        return BIT24;
    }
    if (ipNum == 0xFFFF0000) {
        return BIT16;
    }
    if (ipNum == 0xFF000000) {
        return BIT8;
    }
    for (int32_t i = 0; i < BIT32; i++) {
        if ((ipNum << i) & 0x80000000) {
            cnt++;
        } else {
            break;
        }
    }
    return cnt;
}
} // namespace OHOS::NetManagerStandard::CommonUtils