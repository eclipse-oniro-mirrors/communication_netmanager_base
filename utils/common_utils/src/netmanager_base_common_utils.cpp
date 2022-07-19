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
#include <regex>

namespace OHOS::NetManagerStandard::CommonUtils {
constexpr int32_t INET_OPTION_SUC = 1;
constexpr uint32_t CONST_MASK = 0x80000000;
constexpr size_t MAX_DISPLAY_NUM = 2;

const std::regex IP_PATTERN {
    "((2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)\\.){3}(2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)"
};

const std::regex IP_MASK_PATTERN {
    "((2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)\\.){3}(2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)/(3[0-2]|[1-2]\\d|\\d)"
};

const std::regex IPV6_PATTERN {
    "([\\da-fA-F]{0,4}:){2,7}([\\da-fA-F]{0,4})"
};

const std::regex IPV6_MASK_PATTERN {
    "([\\da-fA-F]{0,4}:){2,7}([\\da-fA-F]{0,4})/(1[0-2][0-8]|[1-9]\\d|[1-9])"
};

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
    auto size = static_cast<int64_t>(str.size());
    int64_t i = 0;
    while (i < size && str[i] == ch) {
        ++i;
    }
    int64_t j = size - 1;
    while (j > 0 && str[j] == ch) {
        --j;
    }
    if (i >= 0 && i < size && j >= 0 && j < size && j - i + 1 > 0) {
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
    int32_t result = inet_pton(AF_INET, ip.c_str(), reinterpret_cast<void *>(&s));
    if (result == INET_OPTION_SUC) {
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
    int32_t result = inet_pton(AF_INET6, ip.c_str(), reinterpret_cast<void *>(&s));
    if (result == INET_OPTION_SUC) {
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

int GetMaskLength(const std::string &mask)
{
    int netMask = 0;
    unsigned int maskTmp = ntohl(static_cast<int>(inet_addr(mask.c_str())));
    while (maskTmp & CONST_MASK) {
        ++netMask;
        maskTmp = (maskTmp << 1);
    }
    return netMask;
}

bool ParseInt(const char *str, int32_t *value)
{
    char *end;
    long long v = strtoll(str, &end, 10);
    if (end == str || *end != '\0' || v < INT_MIN || v > INT_MAX) {
        return false;
    }
    *value = v;
    return true;
}

int64_t ConvertToInt64(const std::string &str)
{
    return strtoll(str.c_str(), nullptr, 10);
}

std::string MakIpv4(std::string &maskedResult)
{
    int maxDisplayNum = MAX_DISPLAY_NUM;
    for (char &i : maskedResult) {
        if (i == '/') {
            break;
        }
        if (maxDisplayNum > 0) {
            if (i == '.') {
                maxDisplayNum--;
            }
        } else {
            if (i != '.') {
                i = '*';
            }
        }
    }
    return maskedResult;
}

std::string MaskIpv6(std::string &maskedResult)
{
    size_t colonCount = 0;
    for (char &i : maskedResult) {
        if (i == '/') {
            break;
        }
        if (i == ':') {
            colonCount++;
        }

        if (colonCount >= MAX_DISPLAY_NUM) { // An legal ipv6 address has at least 2 ':'.
            if (i != ':' && i != '/') {
                i = '*';
            }
        }
    }
    return maskedResult;
}

std::string ToAnonymousIp(const std::string &input)
{
    std::string maskedResult{input};
    // Mask ipv4 address.
    if (std::regex_match(maskedResult, IP_PATTERN) || std::regex_match(maskedResult, IP_MASK_PATTERN)) {
        return MakIpv4(maskedResult);
    }
    // Mask ipv6 address.
    if (std::regex_match(maskedResult, IPV6_PATTERN) || std::regex_match(maskedResult, IPV6_MASK_PATTERN)) {
        return MaskIpv6(maskedResult);
    }
    return input;
}
} // namespace OHOS::NetManagerStandard::CommonUtils
