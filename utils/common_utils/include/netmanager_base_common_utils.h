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

#ifndef COMMUNICATIONNETMANAGER_BASE_COMMON_UTILS_H
#define COMMUNICATIONNETMANAGER_BASE_COMMON_UTILS_H

#include <iosfwd>
#include <sstream>
#include <vector>

namespace OHOS::NetManagerStandard::CommonUtils {
std::vector<std::string> Split(const std::string &str, const std::string &sep);
std::string Strip(const std::string &str, char ch = ' ');
std::string ToLower(const std::string &s);
bool IsValidIPV4(const std::string &ip);
bool IsValidIPV6(const std::string &ip);
int8_t GetAddrFamily(const std::string &ip);
int GetMaskLength(const std::string &mask);
std::string GetMaskByLength(uint32_t length);
std::string ConvertIpv4Address(uint32_t addressIpv4);
uint32_t ConvertIpv4Address(const std::string &address);
int32_t Ipv4PrefixLen(const std::string &ip);
bool ParseInt(const std::string &str, int32_t *value);
int64_t ConvertToInt64(const std::string &str);
std::string ToAnonymousIp(const std::string &input);
int32_t StrToInt(const std::string &str);
uint32_t StrToUint(const std::string &str);
bool StrToBool(const std::string &str);
int64_t StrToLong(const std::string &str);
bool CheckIfaceName(const std::string &name);
int32_t ForkExec(const std::string &command, std::string *out = nullptr);

inline uint64_t GetCurrentSecond()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}
} // namespace OHOS::NetManagerStandard::CommonUtils

#endif /* COMMUNICATIONNETMANAGER_BASE_COMMON_UTILS_H */
