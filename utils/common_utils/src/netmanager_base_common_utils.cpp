/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <netinet/in.h>
#include <regex>
#include <sstream>
#include <set>
#include <string>
#include <sys/socket.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>
#include <vector>
#include <numeric>
#include <fstream>

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "securec.h"

namespace OHOS::NetManagerStandard::CommonUtils {
constexpr int32_t INET_OPTION_SUC = 1;
constexpr int32_t DECIMAL_SYSTEM = 10;
constexpr uint32_t CONST_MASK = 0x80000000;
constexpr size_t MAX_DISPLAY_NUM = 2;
constexpr uint32_t IPV4_DOT_NUM = 3;
constexpr int32_t MIN_BYTE = 0;
constexpr int32_t MAX_BYTE = 255;
constexpr int32_t BYTE_16 = 16;
constexpr uint32_t BIT_NUM_BYTE = 8;
constexpr int32_t BITS_32 = 32;
constexpr int32_t BITS_24 = 24;
constexpr int32_t BITS_16 = 16;
constexpr int32_t BITS_8 = 8;
constexpr uint32_t INTERFACE_NAME_MAX_SIZE = 16;
constexpr int32_t CHAR_ARRAY_SIZE_MAX = 1024;
constexpr int32_t PIPE_FD_NUM = 2;
constexpr int32_t PIPE_OUT = 0;
constexpr int32_t PIPE_IN = 1;
constexpr int32_t DOMAIN_VALID_MIN_PART_SIZE = 2;
constexpr int32_t DOMAIN_VALID_MAX_PART_SIZE = 5;
constexpr int32_t NET_MASK_MAX_LENGTH = 32;
constexpr int32_t NET_MASK_GROUP_COUNT = 4;
constexpr int32_t MAX_IPV6_PREFIX_LENGTH = 128;
const std::string IPADDR_DELIMITER = ".";
constexpr const char *CMD_SEP = " ";
constexpr const char *DOMAIN_DELIMITER = ".";
constexpr const char *TLDS_SPLIT_SYMBOL = "|";
constexpr const char *HOST_DOMAIN_PATTERN_HEADER = "^(https?://)?[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)*\\.(";
constexpr const char *HOST_DOMAIN_PATTERN_TAIL = ")$";
constexpr const char *DEFAULT_IPV6_ANY_INIT_ADDR = "::";
const std::regex IP_PATTERN{
    "((2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)\\.){3}(2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)"};

const std::regex IP_MASK_PATTERN{
    "((2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)\\.){3}(2([0-4]\\d|5[0-5])|1\\d\\d|[1-9]\\d|\\d)/"
    "(3[0-2]|[1-2]\\d|\\d)"};

const std::regex IPV6_PATTERN{"([\\da-fA-F]{0,4}:){2,7}([\\da-fA-F]{0,4})"};

const std::regex IPV6_MASK_PATTERN{"([\\da-fA-F]{0,4}:){2,7}([\\da-fA-F]{0,4})/(1[0-2][0-8]|[1-9]\\d|[1-9])"};

std::vector<std::string> HOST_DOMAIN_TLDS{"com",  "net",     "org",    "edu",  "gov", "mil",  "cn",   "hk",  "tw",
                                          "jp",   "de",      "uk",     "fr",   "au",  "ca",   "br",   "ru",  "it",
                                          "es",   "in",      "online", "shop", "vip", "club", "xyz",  "top", "icu",
                                          "work", "website", "tech",   "asia", "xin", "co",   "mobi", "info"};
std::mutex g_commonUtilsMutex;
std::mutex g_forkExecMutex;

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
    return inet_pton(AF_INET, ip.c_str(), reinterpret_cast<void *>(&s)) == INET_OPTION_SUC;
}

bool IsValidIPV6(const std::string &ip)
{
    if (ip.empty()) {
        return false;
    }
    struct in6_addr s;
    return inet_pton(AF_INET6, ip.c_str(), reinterpret_cast<void *>(&s)) == INET_OPTION_SUC;
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

std::string GetMaskByLength(uint32_t length)
{
    const uint32_t mask = length == 0 ? 0 : 0xFFFFFFFF << (NET_MASK_MAX_LENGTH - length);
    auto maskGroup = new int[NET_MASK_GROUP_COUNT];
    for (int i = 0; i < NET_MASK_GROUP_COUNT; i++) {
        int pos = NET_MASK_GROUP_COUNT - 1 - i;
        maskGroup[pos] = (static_cast<uint32_t>(mask) >> (i * BIT_NUM_BYTE)) & 0x000000ff;
    }
    std::string sMask = "" + std::to_string(maskGroup[0]);
    for (int i = 1; i < NET_MASK_GROUP_COUNT; i++) {
        sMask = sMask + "." + std::to_string(maskGroup[i]);
    }
    delete[] maskGroup;
    return sMask;
}

std::string GetIpv6Prefix(const std::string &ipv6Addr, uint8_t prefixLen)
{
    if (prefixLen >= MAX_IPV6_PREFIX_LENGTH) {
        return ipv6Addr;
    }

    in6_addr ipv6AddrBuf = IN6ADDR_ANY_INIT;
    inet_pton(AF_INET6, ipv6Addr.c_str(), &ipv6AddrBuf);

    char buf[INET6_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET6, &ipv6AddrBuf, buf, INET6_ADDRSTRLEN) == nullptr) {
        return ipv6Addr;
    }

    in6_addr ipv6Prefix = IN6ADDR_ANY_INIT;
    uint32_t byteIndex = prefixLen / BIT_NUM_BYTE;
    if (memset_s(ipv6Prefix.s6_addr, sizeof(ipv6Prefix.s6_addr), 0, sizeof(ipv6Prefix.s6_addr)) != EOK ||
        memcpy_s(ipv6Prefix.s6_addr, sizeof(ipv6Prefix.s6_addr), &ipv6AddrBuf, byteIndex) != EOK) {
        return DEFAULT_IPV6_ANY_INIT_ADDR;
    }
    uint32_t bitOffset = prefixLen & 0x7;
    if ((bitOffset != 0) && (byteIndex < INET_ADDRSTRLEN)) {
        ipv6Prefix.s6_addr[byteIndex] = ipv6AddrBuf.s6_addr[byteIndex] & (0xff00 >> bitOffset);
    }
    char ipv6PrefixBuf[INET6_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET6, &ipv6Prefix, ipv6PrefixBuf, INET6_ADDRSTRLEN);
    return ipv6PrefixBuf;
}

std::string ConvertIpv4Address(uint32_t addressIpv4)
{
    if (addressIpv4 == 0) {
        return "";
    }

    std::ostringstream stream;
    stream << ((addressIpv4 >> BITS_24) & 0xFF) << IPADDR_DELIMITER << ((addressIpv4 >> BITS_16) & 0xFF)
           << IPADDR_DELIMITER << ((addressIpv4 >> BITS_8) & 0xFF) << IPADDR_DELIMITER << (addressIpv4 & 0xFF);
    return stream.str();
}

uint32_t ConvertIpv4Address(const std::string &address)
{
    std::string tmpAddress = address;
    uint32_t addrInt = 0;
    uint32_t i = 0;
    for (i = 0; i < IPV4_DOT_NUM; i++) {
        std::string::size_type npos = tmpAddress.find(IPADDR_DELIMITER);
        if (npos == std::string::npos) {
            break;
        }
        const auto &value = tmpAddress.substr(0, npos);
        int32_t itmp = std::atoi(value.c_str());
        if ((itmp < MIN_BYTE) || (itmp > MAX_BYTE)) {
            break;
        }
        uint32_t utmp = static_cast<uint32_t>(itmp);
        addrInt += utmp << ((IPV4_DOT_NUM - i) * BIT_NUM_BYTE);
        tmpAddress = tmpAddress.substr(npos + 1);
    }

    if (i != IPV4_DOT_NUM) {
        return 0;
    }
    int32_t itmp = std::atoi(tmpAddress.c_str());
    if ((itmp < MIN_BYTE) || (itmp > MAX_BYTE)) {
        return 0;
    }
    uint32_t utmp = static_cast<uint32_t>(itmp);
    addrInt += utmp;

    return addrInt;
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
    ipNum = (c1 << static_cast<uint32_t>(BITS_24)) | (c2 << static_cast<uint32_t>(BITS_16)) |
            (c3 << static_cast<uint32_t>(BITS_8)) | c4;
    if (ipNum == 0xFFFFFFFF) {
        return BITS_32;
    }
    if (ipNum == 0xFFFFFF00) {
        return BITS_24;
    }
    if (ipNum == 0xFFFF0000) {
        return BITS_16;
    }
    if (ipNum == 0xFF000000) {
        return BITS_8;
    }
    for (int32_t i = 0; i < BITS_32; i++) {
        if ((ipNum << i) & 0x80000000) {
            cnt++;
        } else {
            break;
        }
    }
    return cnt;
}

int32_t Ipv6PrefixLen(const std::string &ip)
{
    constexpr int32_t LENGTH_8 = 8;
    constexpr int32_t LENGTH_7 = 7;
    constexpr int32_t LENGTH_6 = 6;
    constexpr int32_t LENGTH_5 = 5;
    constexpr int32_t LENGTH_4 = 4;
    constexpr int32_t LENGTH_3 = 3;
    constexpr int32_t LENGTH_2 = 2;
    constexpr int32_t LENGTH_1 = 1;
    if (ip.empty()) {
        return 0;
    }
    in6_addr addr{};
    inet_pton(AF_INET6, ip.c_str(), &addr);
    int32_t prefixLen = 0;
    for (int32_t i = 0; i < BYTE_16; ++i) {
        if (addr.s6_addr[i] == 0xFF) {
            prefixLen += LENGTH_8;
        } else if (addr.s6_addr[i] == 0xFE) {
            prefixLen += LENGTH_7;
            break;
        } else if (addr.s6_addr[i] == 0xFC) {
            prefixLen += LENGTH_6;
            break;
        } else if (addr.s6_addr[i] == 0xF8) {
            prefixLen += LENGTH_5;
            break;
        } else if (addr.s6_addr[i] == 0xF0) {
            prefixLen += LENGTH_4;
            break;
        } else if (addr.s6_addr[i] == 0xE0) {
            prefixLen += LENGTH_3;
            break;
        } else if (addr.s6_addr[i] == 0xC0) {
            prefixLen += LENGTH_2;
            break;
        } else if (addr.s6_addr[i] == 0x80) {
            prefixLen += LENGTH_1;
            break;
        } else {
            break;
        }
    }
    return prefixLen;
}

bool ParseInt(const std::string &str, int32_t *value)
{
    char *end;
    long long v = strtoll(str.c_str(), &end, 10);
    if (std::string(end) == str || *end != '\0' || v < INT_MIN || v > INT_MAX) {
        return false;
    }
    *value = v;
    return true;
}

int64_t ConvertToInt64(const std::string &str)
{
    return strtoll(str.c_str(), nullptr, DECIMAL_SYSTEM);
}

std::string MaskIpv4(std::string &maskedResult)
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
    std::lock_guard<std::mutex> lock(g_commonUtilsMutex);
    std::string maskedResult{input};
    // Mask ipv4 address.
    if (std::regex_match(maskedResult, IP_PATTERN) || std::regex_match(maskedResult, IP_MASK_PATTERN)) {
        return MaskIpv4(maskedResult);
    }
    // Mask ipv6 address.
    if (std::regex_match(maskedResult, IPV6_PATTERN) || std::regex_match(maskedResult, IPV6_MASK_PATTERN)) {
        return MaskIpv6(maskedResult);
    }
    return input;
}

int32_t StrToInt(const std::string &value, int32_t defaultErr)
{
    errno = 0;
    char *pEnd = nullptr;
    int64_t result = std::strtol(value.c_str(), &pEnd, 0);
    if (pEnd == value.c_str() || (result < INT_MIN || result > LONG_MAX) || errno == ERANGE) {
        return defaultErr;
    }
    return result;
}

uint32_t StrToUint(const std::string &value, uint32_t defaultErr)
{
    errno = 0;
    char *pEnd = nullptr;
    uint64_t result = std::strtoul(value.c_str(), &pEnd, 0);
    if (pEnd == value.c_str() || result > UINT32_MAX || errno == ERANGE) {
        return defaultErr;
    }
    return result;
}

bool StrToBool(const std::string &value, bool defaultErr)
{
    errno = 0;
    char *pEnd = nullptr;
    uint64_t result = std::strtoul(value.c_str(), &pEnd, 0);
    if (pEnd == value.c_str() || result > UINT32_MAX || errno == ERANGE) {
        return defaultErr;
    }
    return static_cast<bool>(result);
}

int64_t StrToLong(const std::string &value, int64_t defaultErr)
{
    errno = 0;
    char *pEnd = nullptr;
    int64_t result = std::strtoll(value.c_str(), &pEnd, 0);
    if (pEnd == value.c_str() || errno == ERANGE) {
        return defaultErr;
    }
    return result;
}

uint64_t StrToUint64(const std::string &value, uint64_t defaultErr)
{
    errno = 0;
    char *pEnd = nullptr;
    uint64_t result = std::strtoull(value.c_str(), &pEnd, 0);
    if (pEnd == value.c_str() || errno == ERANGE) {
        return defaultErr;
    }
    return result;
}

bool CheckIfaceName(const std::string &name)
{
    uint32_t index = 0;
    if (name.empty()) {
        return false;
    }
    size_t len = name.size();
    if (len > INTERFACE_NAME_MAX_SIZE) {
        return false;
    }
    while (index < len) {
        if ((index == 0) && !isalnum(name[index])) {
            return false;
        }
        if (!isalnum(name[index]) && (name[index] != '-') && (name[index] != '_') && (name[index] != '.') &&
            (name[index] != ':')) {
            return false;
        }
        index++;
    }
    return true;
}

std::vector<const char *> FormatCmd(const std::vector<std::string> &cmd)
{
    std::vector<const char *> res;
    res.reserve(cmd.size() + 1);

    // string is converted to char * and the result is saved in res
    std::transform(cmd.begin(), cmd.end(), std::back_inserter(res), [](const std::string &str) { return str.c_str(); });
    res.emplace_back(nullptr);
    return res;
}

int32_t ForkExecChildProcess(const int32_t *pipeFd, int32_t count, const std::vector<const char *> &args)
{
    if (count != PIPE_FD_NUM) {
        NETMGR_LOG_E("fork exec parent process failed");
        _exit(-1);
    }
    if (close(pipeFd[PIPE_OUT]) != 0) {
        NETMGR_LOG_E("close failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
        _exit(-1);
    }
    if (dup2(pipeFd[PIPE_IN], STDOUT_FILENO) == -1) {
        NETMGR_LOG_E("dup2 failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
        _exit(-1);
    }
    if (execv(args[0], const_cast<char *const *>(&args[0])) == -1) {
        NETMGR_LOG_E("execv command failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
    }
    if (close(pipeFd[PIPE_IN]) != 0) {
        NETMGR_LOG_E("close failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
        _exit(-1);
    }
    _exit(-1);
}

int32_t ForkExecParentProcess(const int32_t *pipeFd, int32_t count, pid_t childPid, std::string *out)
{
    if (count != PIPE_FD_NUM) {
        NETMGR_LOG_E("fork exec parent process failed");
        return NETMANAGER_ERROR;
    }
    if (out != nullptr) {
        char buf[CHAR_ARRAY_SIZE_MAX] = {0};
        out->clear();
        if (close(pipeFd[PIPE_IN]) != 0) {
            NETMGR_LOG_E("close failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
        }
        while (read(pipeFd[PIPE_OUT], buf, CHAR_ARRAY_SIZE_MAX - 1) > 0) {
            out->append(buf);
            if (memset_s(buf, sizeof(buf), 0, sizeof(buf)) != 0) {
                NETMGR_LOG_E("memset is false");
                close(pipeFd[PIPE_OUT]);
                return NETMANAGER_ERROR;
            }
        }
        if (close(pipeFd[PIPE_OUT]) != 0) {
            NETMGR_LOG_E("close failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
            _exit(-1);
        }
        return NETMANAGER_SUCCESS;
    } else {
        NETMGR_LOG_D("there is no need to return execution results");
        close(pipeFd[PIPE_IN]);
        close(pipeFd[PIPE_OUT]);
    }
    pid_t pidRet = waitpid(childPid, nullptr, 0);
    if (pidRet != childPid) {
        NETMGR_LOG_E("waitpid[%{public}d] failed, pidRet:%{public}d", childPid, pidRet);
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}

int32_t ForkExec(const std::string &command, std::string *out)
{
    std::unique_lock<std::mutex> lock(g_forkExecMutex);
    const std::vector<std::string> cmd = Split(command, CMD_SEP);
    std::vector<const char *> args = FormatCmd(cmd);
    int32_t pipeFd[PIPE_FD_NUM] = {0};
    if (pipe(pipeFd) < 0) {
        NETMGR_LOG_E("creat pipe failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERROR;
    }
    pid_t pid = fork();
    if (pid < 0) {
        NETMGR_LOG_E("fork failed, errorno:%{public}d, errormsg:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERROR;
    }
    if (pid == 0) {
        ForkExecChildProcess(pipeFd, PIPE_FD_NUM, args);
        return NETMANAGER_SUCCESS;
    } else {
        return ForkExecParentProcess(pipeFd, PIPE_FD_NUM, pid, out);
    }
}

bool IsValidDomain(const std::string &domain)
{
    if (domain.empty()) {
        return false;
    }

    std::string pattern = HOST_DOMAIN_PATTERN_HEADER;
    pattern = std::accumulate(HOST_DOMAIN_TLDS.begin(), HOST_DOMAIN_TLDS.end(), pattern,
        [](const std::string &pattern, const std::string &tlds) { return pattern + tlds + TLDS_SPLIT_SYMBOL; });
    pattern = pattern.replace(pattern.size() - 1, 1, "") + HOST_DOMAIN_PATTERN_TAIL;
    std::regex reg(pattern);
    if (!std::regex_match(domain, reg)) {
        NETMGR_LOG_E("Domain:%{public}s regex match failed.", domain.c_str());
        return false;
    }

    std::vector<std::string> parts = Split(domain, DOMAIN_DELIMITER);
    if (parts.size() < DOMAIN_VALID_MIN_PART_SIZE || parts.size() > DOMAIN_VALID_MAX_PART_SIZE) {
        NETMGR_LOG_E("The domain:[%{public}s] parts size:[%{public}d] is invalid", domain.c_str(),
                     static_cast<int>(parts.size()));
        return false;
    }

    std::set<std::string> tldsList;
    for (const auto &item : parts) {
        if (std::find(HOST_DOMAIN_TLDS.begin(), HOST_DOMAIN_TLDS.end(), item) == HOST_DOMAIN_TLDS.end()) {
            continue;
        }
        if (tldsList.find(item) != tldsList.end()) {
            NETMGR_LOG_E("Domain:%{public}s has duplicate tlds:%{public}s", domain.c_str(), item.c_str());
            return false;
        }
        tldsList.insert(item);
    }
    return true;
}

bool WriteFile(const std::string &filePath, const std::string &fileContent)
{
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        NETMGR_LOG_E("write file=%{public}s fstream failed. err %{public}d %{public}s",
            filePath.c_str(), errno, strerror(errno));
        return false;
    }
    file << fileContent;
    file.close();
    return true;
}

bool HasInternetPermission()
{
    int testSock = socket(AF_INET, SOCK_STREAM, 0);
    if (testSock < 0 && errno == EPERM) {
        NETMGR_LOG_E("make tcp testSock failed errno is %{public}d %{public}s", errno, strerror(errno));
        return false;
    }
    if (testSock > 0) {
        close(testSock);
    }
    return true;
}

std::string Trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

bool IsUrlRegexValid(const std::string &regex)
{
    if (Trim(regex).empty()) {
        return false;
    }
    return regex_match(regex, std::regex("^[a-zA-Z0-9\\-_\\.*]+$"));
}

std::string InsertCharBefore(const std::string &input, const char from, const char preChar, const char nextChar)
{
    std::ostringstream output;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == from && (i == input.size() - 1 || input[i + 1] != nextChar)) {
            output << preChar;
        }
        output << input[i];
    }
    return output.str();
}

std::string ReplaceCharacters(const std::string &input)
{
    std::string output = InsertCharBefore(input, '*', '.', '\0');
    output = InsertCharBefore(output, '.', '\\', '*');
    return output;
}

bool UrlRegexParse(const std::string &str, const std::string &patternStr)
{
    if (patternStr.empty()) {
        return false;
    }
    if (patternStr == "*") {
        return true;
    }
    if (!IsUrlRegexValid(patternStr)) {
        return patternStr == str;
    }
    std::regex pattern(ReplaceCharacters(patternStr));
    return !patternStr.empty() && std::regex_match(str, pattern);
}
} // namespace OHOS::NetManagerStandard::CommonUtils
