/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef COMMUNICATIONNETMANAGER_BASE_COMMON_UTILS_H
#define COMMUNICATIONNETMANAGER_BASE_COMMON_UTILS_H

#include <iosfwd>
#include <sstream>
#include <vector>

namespace OHOS::NetManagerStandard {
constexpr uint32_t ROUTE_INTERNAL_DEFAULT_TABLE = 10;
constexpr uint32_t INVALID_NET_ID = 0;
constexpr int32_t MIN_INTERNAL_NET_ID = 1;
constexpr int32_t MAX_INTERNAL_NET_ID = 50;
constexpr int32_t MIN_NET_ID = 100;
constexpr int32_t MAX_NET_ID = 0xFFFF - 0x400;
inline bool IsInternalNetId(int32_t netId)
{
    return netId >= MIN_INTERNAL_NET_ID && netId <= MAX_INTERNAL_NET_ID;
}

inline uint32_t ConvertTableByNetId(int32_t netId, uint32_t table)
{
    return IsInternalNetId(netId) ? table % ROUTE_INTERNAL_DEFAULT_TABLE + 1 : table;
}
}
namespace OHOS::NetManagerStandard::CommonUtils {
inline std::vector<std::string> Split(const std::string &str, const std::string &sep)
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
std::string Strip(const std::string &str, char ch = ' ');
std::string ToLower(const std::string &s);
bool IsValidIPV4(const std::string &ip);
bool IsValidIPV6(const std::string &ip);
int8_t GetAddrFamily(const std::string &ip);
int GetMaskLength(const std::string &mask);
std::string GetMaskByLength(uint32_t length);
std::string GetIpv6Prefix(const std::string &ipv6Addr, uint8_t prefixLen);
std::string ConvertIpv4Address(uint32_t addressIpv4);
uint32_t ConvertIpv4Address(const std::string &address);
int32_t Ipv4PrefixLen(const std::string &ip);
int32_t Ipv6PrefixLen(const std::string &ip);
bool ParseInt(const std::string &str, int32_t *value);
int64_t ConvertToInt64(const std::string &str);
std::string ToAnonymousIp(const std::string &input);
std::string AnonymizeIptablesCommand(const std::string &command);
std::string AnonymousIpInStr(const std::string &input);
int32_t StrToInt(const std::string &value, int32_t defaultErr = -1);
uint32_t StrToUint(const std::string &value, uint32_t defaultErr = 0);
bool StrToBool(const std::string &value, bool defaultErr = false);
int64_t StrToLong(const std::string &value, int64_t defaultErr = -1);
uint64_t StrToUint64(const std::string &value, uint64_t defaultErr = 0);
bool CheckIfaceName(const std::string &name);
int32_t ForkExec(const std::string &command, std::string *out = nullptr);
bool IsValidDomain(const std::string &domain);
bool HasInternetPermission();
std::string Trim(const std::string &str);
bool IsUrlRegexValid(const std::string &regex);
std::string InsertCharBefore(const std::string &input, const char from, const char preChar, const char nextChar);
std::string ReplaceCharacters(const std::string &input);
bool UrlRegexParse(const std::string &str, const std::string &patternStr);
uint64_t GenRandomNumber();
bool IsSim(const std::string &bundleName);
bool IsInstallSourceFromSim(const std::string &installSource);
bool IsSim2(const std::string &bundleName);
bool IsInstallSourceFromSim2(const std::string &installSource);
bool IsSimAnco(const std::string &bundleName);
bool IsSim2Anco(const std::string &bundleName);

inline uint64_t GetCurrentSecond()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

inline bool IsSameNaturalDay(uint32_t current, uint32_t another)
{
    std::time_t tt1 =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(std::chrono::seconds(current)));
    std::time_t tt2 =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(std::chrono::seconds(another)));
    std::tm *tm1 = std::localtime(&tt1);
    std::tm *tm2 = std::localtime(&tt2);
    if (tm1 == nullptr || tm2 == nullptr) {
        return false;
    }
    return tm1->tm_year == tm2->tm_year && tm1->tm_mon == tm2->tm_mon && tm1->tm_mday == tm2->tm_mday;
}

bool WriteFile(const std::string &filePath, const std::string &fileContent);
std::string GetHostnameFromURL(const std::string &url);
} // namespace OHOS::NetManagerStandard::CommonUtils

#endif /* COMMUNICATIONNETMANAGER_BASE_COMMON_UTILS_H */
