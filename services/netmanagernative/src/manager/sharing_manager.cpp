/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "sharing_manager.h"

#include <cerrno>
#include <fcntl.h>
#include <regex>
#include <unistd.h>
#include <charconv>

#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
namespace {
constexpr const char *IPV4_FORWARDING_PROC_FILE = "/proc/sys/net/ipv4/ip_forward";
constexpr const char *IPV6_FORWARDING_PROC_FILE = "/proc/sys/net/ipv6/conf/all/forwarding";
constexpr const char *IPTABLES_TMP_BAK = "/data/service/el1/public/netmanager/ipfwd.bak";
constexpr const char *IPV6_PROC_PATH = "/proc/sys/net/ipv6/conf/";
constexpr const char *IP6TABLES_TMP_BAK = "/data/service/el1/public/netmanager/ip6fwd.bak";
constexpr const int MAX_MATCH_SIZE = 4;
constexpr const int TWO_LIST_CORRECT_DATA = 2;
constexpr const int NEXT_LIST_CORRECT_DATA = 1;
constexpr uint32_t NET_TRAFFIC_RESULT_INDEX_OFFSET = 2;
const std::string CELLULAR_IFACE_NAME = "rmnet";
const std::string WLAN_IFACE_NAME = "wlan";
const std::string P2P_IFACE_NAME = "p2p-p2p0";

// commands of create tables
constexpr const char *CREATE_TETHERCTRL_NAT_POSTROUTING = "-t nat -N tetherctrl_nat_POSTROUTING";
constexpr const char *CREATE_TETHERCTRL_FORWARD = "-t filter -N tetherctrl_FORWARD";
constexpr const char *CREATE_TETHERCTRL_COUNTERS = "-t filter -N tetherctrl_counters";
constexpr const char *CREATE_TETHERCTRL_MANGLE_FORWARD = "-t mangle -N tetherctrl_mangle_FORWARD";
constexpr const char *OPEN_IPV6_PRIVACY_EXTENSIONS = "2";
constexpr const char *CLOSE_IPV6_PRIVACY_EXTENSIONS = "0";
constexpr const char *ENABLE_IPV6_VALUE = "0";
constexpr const char *DISABLE_IPV6_VALUE = "1";

// commands of set nat
constexpr const char *APPEND_NAT_POSTROUTING = "-A POSTROUTING -j tetherctrl_nat_POSTROUTING";
constexpr const char *APPEND_MANGLE_FORWARD = "-A FORWARD -j tetherctrl_mangle_FORWARD";
constexpr const char *APPEND_TETHERCTRL_MANGLE_FORWARD =
    "-A tetherctrl_mangle_FORWARD "
    "-p tcp -m tcp --tcp-flags SYN SYN -j TCPMSS --clamp-mss-to-pmtu";
constexpr const char *CLEAR_TETHERCTRL_NAT_POSTROUTING = "-F tetherctrl_nat_POSTROUTING";
constexpr const char *CLEAR_TETHERCTRL_MANGLE_FORWARD = "-F tetherctrl_mangle_FORWARD";
constexpr const char *DELETE_TETHERCTRL_NAT_POSTROUTING = "-D POSTROUTING -j tetherctrl_nat_POSTROUTING";
constexpr const char *DELETE_TETHERCTRL_MANGLE_FORWARD = "-D FORWARD -j tetherctrl_mangle_FORWARD";

constexpr const char *IPATBLES_RESTORE_CMD_PATH = "/system/bin/iptables-restore";
constexpr const char *IPATBLES_SAVE_CMD_PATH = "/system/bin/iptables-save";

constexpr const char *IP6ATBLES_RESTORE_CMD_PATH = "/system/bin/ip6tables-restore";
constexpr const char *IP6ATBLES_SAVE_CMD_PATH = "/system/bin/ip6tables-save";

constexpr const char *FILTER_TABLE = "*filter";
constexpr const char *MANGLE_TABLE = "*mangle";
constexpr const char *NAT_TABLE = "*nat";
constexpr const char *CMD_COMMIT = "COMMIT";

const std::string EnableNatCmd(const std::string &down)
{
    return "-A tetherctrl_nat_POSTROUTING -o " + down + " -j MASQUERADE";
}

// commands of set ipfwd, all commands with filter
constexpr const char *FORWARD_JUMP_TETHERCTRL_FORWARD = " FORWARD -j tetherctrl_FORWARD";
constexpr const char *SET_TETHERCTRL_FORWARD_DROP = " tetherctrl_FORWARD -j DROP";
const std::string SetTetherctrlForward1(const std::string &from, const std::string &to)
{
    return " tetherctrl_FORWARD -i " + from + " -o " + to +
           " -m state --state RELATED,ESTABLISHED"
           " -g tetherctrl_counters";
}

const std::string SetTetherctrlForward2(const std::string &from, const std::string &to)
{
    return " tetherctrl_FORWARD -i " + to + " -o " + from + " -m state --state INVALID -j DROP";
}

const std::string SetTetherctrlForward3(const std::string &from, const std::string &to)
{
    return " tetherctrl_FORWARD -i " + to + " -o " + from + " -g tetherctrl_counters";
}

const std::string SetTetherctrlCounters1(const std::string &from, const std::string &to)
{
    return " tetherctrl_counters -i " + to + " -o " + from + " -j RETURN";
}

const std::string SetTetherctrlCounters2(const std::string &from, const std::string &to)
{
    return " tetherctrl_counters -i " + from + " -o " + to + " -j RETURN";
}

bool WriteToFile(const char *fileName, const char *value)
{
    if (fileName == nullptr) {
        return false;
    }
    int fd = open(fileName, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        NETNATIVE_LOGE("failed to open file: %{public}s", strerror(errno));
        return false;
    }

    const ssize_t len = strlen(value);
    if (write(fd, value, len) != len) {
        NETNATIVE_LOGE("faield to write %{public}s: %{public}s", value, strerror(errno));
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

void Rollback()
{
    NETNATIVE_LOGE("iptables rollback");
    std::string rollBak = std::string(IPATBLES_RESTORE_CMD_PATH) + " -T filter < ";
    rollBak.append(IPTABLES_TMP_BAK);
    CommonUtils::ForkExec(rollBak);

    rollBak = std::string(IP6ATBLES_RESTORE_CMD_PATH) + " -T filter < ";
    rollBak.append(IP6TABLES_TMP_BAK);
    CommonUtils::ForkExec(rollBak);
}
} // namespace

SharingManager::SharingManager()
{
    iptablesWrapper_ = IptablesWrapper::GetInstance();
}

void SharingManager::InitChildChains()
{
    iptablesWrapper_->RunCommand(IPTYPE_IPV4V6, CREATE_TETHERCTRL_NAT_POSTROUTING);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4V6, CREATE_TETHERCTRL_FORWARD);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4V6, CREATE_TETHERCTRL_COUNTERS);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4V6, CREATE_TETHERCTRL_MANGLE_FORWARD);
    inited_ = true;
}

int32_t SharingManager::IpEnableForwarding(const std::string &requestor)
{
    NETNATIVE_LOG_D("IpEnableForwarding requestor: %{public}s", requestor.c_str());
    {
        std::lock_guard<std::mutex> guard(initedMutex_);
        forwardingRequests_.insert(requestor);
    }
    return SetIpFwdEnable();
}

int32_t SharingManager::IpDisableForwarding(const std::string &requestor)
{
    NETNATIVE_LOG_D("IpDisableForwarding requestor: %{public}s", requestor.c_str());
    {
        std::lock_guard<std::mutex> guard(initedMutex_);
        forwardingRequests_.erase(requestor);
    }
    return SetIpFwdEnable();
}

int32_t SharingManager::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    DisableNat(downstreamIface, upstreamIface);
    CheckInited();
    if (downstreamIface == upstreamIface) {
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %{public}s", downstreamIface.c_str(),
                       upstreamIface.c_str());
        return -1;
    }
    if (!CommonUtils::CheckIfaceName(upstreamIface)) {
        NETNATIVE_LOGE("iface name valid check fail: %{public}s", upstreamIface.c_str());
        return -1;
    }

    NETNATIVE_LOGI("EnableNat downstreamIface: %{public}s, upstreamIface: %{public}s", downstreamIface.c_str(),
                   upstreamIface.c_str());

    std::string cmdSet = "";
    CombineRestoreRules(NAT_TABLE, cmdSet);
    CombineRestoreRules(APPEND_NAT_POSTROUTING, cmdSet);
    CombineRestoreRules(EnableNatCmd(upstreamIface), cmdSet);
    CombineRestoreRules(CMD_COMMIT, cmdSet);
    
    CombineRestoreRules(MANGLE_TABLE, cmdSet);
    CombineRestoreRules(APPEND_MANGLE_FORWARD, cmdSet);
    CombineRestoreRules(APPEND_TETHERCTRL_MANGLE_FORWARD, cmdSet);
    CombineRestoreRules(CMD_COMMIT, cmdSet);

    if (iptablesWrapper_->RunRestoreCommands(IPTYPE_IPV4V6, cmdSet) !=
        NetManagerStandard::NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("IptablesWrapper run command failed");
        return -1;
    }
    return 0;
}

int32_t SharingManager::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    CheckInited();
    if (downstreamIface == upstreamIface) {
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %s", downstreamIface.c_str(), upstreamIface.c_str());
        return -1;
    }
    if (!CommonUtils::CheckIfaceName(upstreamIface)) {
        NETNATIVE_LOGE("iface name valid check fail: %{public}s", upstreamIface.c_str());
        return -1;
    }

    NETNATIVE_LOGI("DisableNat downstreamIface: %{public}s, upstreamIface: %{public}s", downstreamIface.c_str(),
                   upstreamIface.c_str());

    std::string cmdSet = "";
    CombineRestoreRules(NAT_TABLE, cmdSet);
    CombineRestoreRules(CLEAR_TETHERCTRL_NAT_POSTROUTING, cmdSet);
    CombineRestoreRules(DELETE_TETHERCTRL_NAT_POSTROUTING, cmdSet);
    CombineRestoreRules(CMD_COMMIT, cmdSet);
    
    CombineRestoreRules(MANGLE_TABLE, cmdSet);
    CombineRestoreRules(CLEAR_TETHERCTRL_MANGLE_FORWARD, cmdSet);
    CombineRestoreRules(DELETE_TETHERCTRL_MANGLE_FORWARD, cmdSet);
    CombineRestoreRules(CMD_COMMIT, cmdSet);

    if (iptablesWrapper_->RunRestoreCommands(IPTYPE_IPV4V6, cmdSet) !=
        NetManagerStandard::NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("IptablesWrapper run command failed");
        return -1;
    }
    return 0;
}
int32_t SharingManager::SetIpv6PrivacyExtensions(const std::string &interfaceName, const uint32_t on)
{
    std::string option = IPV6_PROC_PATH + interfaceName + "/use_tempaddr";
    const char *value = on ? OPEN_IPV6_PRIVACY_EXTENSIONS : CLOSE_IPV6_PRIVACY_EXTENSIONS;
    bool ipv6Success = WriteToFile(option.c_str(), value);
    return ipv6Success ? 0 : -1;
}

int32_t SharingManager::SetEnableIpv6(const std::string &interfaceName, const uint32_t on, bool needRestart)
{
    std::string option = IPV6_PROC_PATH + interfaceName + "/disable_ipv6";
    const char *value = on ? ENABLE_IPV6_VALUE : DISABLE_IPV6_VALUE;
    bool ipv6Success = WriteToFile(option.c_str(), value);
    std::lock_guard<std::mutex> guard(enableV6mutex_);
    bool isIfEnabled = true;
    if (enableV6Map_.find(interfaceName) != enableV6Map_.end()) {
        isIfEnabled = enableV6Map_[interfaceName];
    }
    if (!on && needRestart && isIfEnabled) {
        bool restartSuccess = WriteToFile(option.c_str(), ENABLE_IPV6_VALUE);
        NETMGR_LOG_I("SetEnableIpv6: interfaceName=%{public}s, restart V6, result = %{public}d",
            interfaceName.c_str(), restartSuccess);
        enableV6Map_[interfaceName] = true;
        return restartSuccess ? 0 : -1;
    }
    enableV6Map_[interfaceName] = static_cast<bool>(on);
    return ipv6Success ? 0 : -1;
}

int32_t SharingManager::SetIpFwdEnable()
{
    bool disable = forwardingRequests_.empty();
    const char *value = disable ? "0" : "1";
    bool ipv4Success = WriteToFile(IPV4_FORWARDING_PROC_FILE, value);
    bool ipv6Success = WriteToFile(IPV6_FORWARDING_PROC_FILE, value);
    return (ipv4Success && ipv6Success) ? 0 : -1;
}

void SharingManager::IpfwdExecSaveBak()
{
    std::string saveBak = std::string(IPATBLES_SAVE_CMD_PATH) + " -t filter > ";
    saveBak.append(IPTABLES_TMP_BAK);
    CommonUtils::ForkExec(saveBak);
    saveBak = std::string(IP6ATBLES_SAVE_CMD_PATH) + " -t filter > ";
    saveBak.append(IP6TABLES_TMP_BAK);
    CommonUtils::ForkExec(saveBak);
}

int32_t SharingManager::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    CheckInited();
    if (fromIface == toIface) {
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %{public}s", fromIface.c_str(), toIface.c_str());
        return -1;
    }
    if (!(CommonUtils::CheckIfaceName(fromIface)) || !(CommonUtils::CheckIfaceName(toIface))) {
        NETNATIVE_LOGE("iface name valid check fail: %{public}s %{public}s", fromIface.c_str(), toIface.c_str());
        return -1;
    }
    NETNATIVE_LOGI("IpfwdAddInterfaceForward fromIface: %{public}s, toIface: %{public}s", fromIface.c_str(),
                   toIface.c_str());

    IpfwdExecSaveBak();
    int32_t result = 0;

    std::string fwdCmdSet = "";
    CombineRestoreRules(FILTER_TABLE, fwdCmdSet);
    if (interfaceForwards_.empty()) {
        SetForwardRules(true, FORWARD_JUMP_TETHERCTRL_FORWARD, fwdCmdSet);
    }
    /*
     * Add a forward rule, when the status of packets is RELATED,
     * ESTABLISED and from fromIface to toIface, goto tetherctrl_counters
     */
    SetForwardRules(true, SetTetherctrlForward1(toIface, fromIface), fwdCmdSet);

    /*
     * Add a forward rule, when the status is INVALID and from toIface to fromIface, just drop
     */
    SetForwardRules(true, SetTetherctrlForward2(toIface, fromIface), fwdCmdSet);

    /*
     * Add a forward rule, from toIface to fromIface, goto tetherctrl_counters
     */
    SetForwardRules(true, SetTetherctrlForward3(toIface, fromIface), fwdCmdSet);

    if (!interfaceForwards_.empty()) {
        // ensure only one drop rule
        SetForwardRules(false, SET_TETHERCTRL_FORWARD_DROP, fwdCmdSet);
    }

    /*
     * Add a forward rule, drop others
     */
    SetForwardRules(true, SET_TETHERCTRL_FORWARD_DROP, fwdCmdSet);

    /*
     * Add a forward rule, if from toIface to fromIface return chain of father
     */
    SetForwardRules(true, SetTetherctrlCounters1(fromIface, toIface), fwdCmdSet);

    /*
     * Add a forward rule, if from fromIface to toIface return chain of father
     */
    SetForwardRules(true, SetTetherctrlCounters2(fromIface, toIface), fwdCmdSet);

    CombineRestoreRules(CMD_COMMIT, fwdCmdSet);
    if (iptablesWrapper_->RunRestoreCommands(IPTYPE_IPV4V6, fwdCmdSet) !=
        NetManagerStandard::NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("IptablesWrapper run command failed");
        Rollback();
        return -1;
    }

    if (RouteManager::EnableSharing(fromIface, toIface)) {
        Rollback();
        return result;
    }
    if (fromIface.find(WLAN_IFACE_NAME) != std::string::npos ||
        fromIface.find(P2P_IFACE_NAME) != std::string::npos) {
        wifiShareInterface_ = fromIface;
        EnableShareUnreachableRoute(RouteManager::UNREACHABLE_NETWORK);
    }
    interfaceForwards_.insert(fromIface + toIface);
    return 0;
}

int32_t SharingManager::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    CheckInited();
    if (fromIface == toIface) {
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %{public}s", fromIface.c_str(), toIface.c_str());
        return -1;
    }
    if (!(CommonUtils::CheckIfaceName(fromIface)) || !(CommonUtils::CheckIfaceName(toIface))) {
        NETNATIVE_LOGE("iface name valid check fail: %{public}s %{public}s", fromIface.c_str(), toIface.c_str());
        return -1;
    }
    NETNATIVE_LOGI("IpfwdRemoveInterfaceForward fromIface: %{public}s, toIface: %{public}s", fromIface.c_str(),
                   toIface.c_str());

    std::string fwdCmdSet = "";
    CombineRestoreRules(FILTER_TABLE, fwdCmdSet);
    SetForwardRules(false, SetTetherctrlForward1(toIface, fromIface), fwdCmdSet);
    SetForwardRules(false, SetTetherctrlForward2(toIface, fromIface), fwdCmdSet);
    SetForwardRules(false, SetTetherctrlForward3(toIface, fromIface), fwdCmdSet);
    SetForwardRules(false, SetTetherctrlCounters1(fromIface, toIface), fwdCmdSet);
    SetForwardRules(false, SetTetherctrlCounters2(fromIface, toIface), fwdCmdSet);

    interfaceForwards_.erase(fromIface + toIface);
    if (interfaceForwards_.empty()) {
        SetForwardRules(false, SET_TETHERCTRL_FORWARD_DROP, fwdCmdSet);
        SetForwardRules(false, FORWARD_JUMP_TETHERCTRL_FORWARD, fwdCmdSet);
    }

    CombineRestoreRules(CMD_COMMIT, fwdCmdSet);
    iptablesWrapper_->RunRestoreCommands(IPTYPE_IPV4V6, fwdCmdSet);

    RouteManager::DisableSharing(fromIface, toIface);

    if (fromIface.find(WLAN_IFACE_NAME) != std::string::npos ||
        fromIface.find(P2P_IFACE_NAME) != std::string::npos) {
        ClearForbidIpRules();
        DisableShareUnreachableRoute(RouteManager::UNREACHABLE_NETWORK);
        wifiShareInterface_ = "";
    }
    return 0;
}

int32_t SharingManager::GetNetworkSharingTraffic(const std::string &downIface, const std::string &upIface,
                                                 NetworkSharingTraffic &traffic)
{
    const std::string cmds = "-t filter -L tetherctrl_counters -nvx";
    std::string result = iptablesWrapper_->RunCommandForRes(IPTYPE_IPV4V6, cmds);
    const std::string num = "(\\d+)";
    const std::string iface = "([^\\s]+)";
    const std::string dst = "(0.0.0.0/0|::/0)";
    const std::string counters = "\\s*" + num + "\\s+" + num + " RETURN     all(  --  |      )" + iface + "\\s+" +
                                 iface + "\\s+" + dst + "\\s+" + dst;
    static const std::regex IP_RE(counters);

    bool isFindTx = false;
    bool isFindRx = false;
    const std::vector<std::string> lines = CommonUtils::Split(result, "\n");
    std::size_t size1 = lines.size();
    for (auto line : lines) {
        std::smatch matches;
        std::regex_search(line, matches, IP_RE);
        if (matches.size() < MAX_MATCH_SIZE) {
            continue;
        }
        for (uint32_t i = 0; i < matches.size() - 1; i++) {
            std::string tempMatch = matches[i];
            NETNATIVE_LOG_D("GetNetworkSharingTraffic matche[%{public}s]", tempMatch.c_str());
            if (matches[i] == downIface && matches[i + NEXT_LIST_CORRECT_DATA] == upIface &&
                ((i - TWO_LIST_CORRECT_DATA) >= 0)) {
                int64_t send =
                    static_cast<int64_t>(strtoul(matches[i - TWO_LIST_CORRECT_DATA].str().c_str(), nullptr, 0));
                isFindTx = true;
                traffic.send = send;
                traffic.all += send;
            } else if (matches[i] == upIface && matches[i + NEXT_LIST_CORRECT_DATA] == downIface &&
                       ((i - NET_TRAFFIC_RESULT_INDEX_OFFSET) >= 0)) {
                int64_t receive =
                    static_cast<int64_t>(strtoul(matches[i - TWO_LIST_CORRECT_DATA].str().c_str(), nullptr, 0));
                isFindRx = true;
                traffic.receive = receive;
                traffic.all += receive;
            }
            if (isFindTx && isFindRx) {
                NETNATIVE_LOG_D("GetNetworkSharingTraffic success total");
                return NETMANAGER_SUCCESS;
            }
        }
    }
    NETNATIVE_LOGE("GetNetworkSharingTraffic failed");
    return NETMANAGER_ERROR;
}

int32_t SharingManager::GetNetworkCellularSharingTraffic(NetworkSharingTraffic &traffic, std::string &ifaceName)
{
    const std::string cmds = "-t filter -L tetherctrl_counters -nvx";
    for (IpType ipType : {IPTYPE_IPV4, IPTYPE_IPV6}) {
        NetworkSharingTraffic traffic0;
        std::string ifaceName0 = "";
        std::string result = iptablesWrapper_->RunCommandForRes(ipType, cmds);
        int32_t ret = QueryCellularSharingTraffic(traffic0, result, ifaceName0);
        if (ret != NETMANAGER_SUCCESS) {
            NETNATIVE_LOGI("ipv4 GetNetworkSharingTraffic failed");
            return NETMANAGER_ERROR;
        }
        traffic.receive += traffic0.receive;
        traffic.send += traffic0.send;
        traffic.all += traffic0.all;
        ifaceName = ifaceName0;
    }
    NETNATIVE_LOG_D("GetNetworkCellularSharingTraffic success");
    return NETMANAGER_SUCCESS;
}

int32_t SharingManager::QueryCellularSharingTraffic(NetworkSharingTraffic &traffic,
    const std::string &result, std::string &ifaceName)
{
    const std::string num = "(\\d+)";
    const std::string iface = "([^\\s]+)";
    const std::string dst = "(0.0.0.0/0|::/0)";
    const std::string counters = "\\s*" + num + "\\s+" + num + " RETURN     all(  --  |      )" + iface + "\\s+" +
                                 iface + "\\s+" + dst + "\\s+" + dst;
    static const std::regex IP_RE(counters);

    bool isFindTx = false;
    bool isFindRx = false;
    const std::vector<std::string> lines = CommonUtils::Split(result, "\n");
    for (auto line : lines) {
        std::smatch matches;
        std::regex_search(line, matches, IP_RE);
        if (matches.size() < MAX_MATCH_SIZE) {
            continue;
        }
        GetTraffic(matches, ifaceName, traffic, isFindTx, isFindRx);
        if (isFindTx && isFindRx) {
            NETNATIVE_LOG_D("GetNetworkSharingTraffic success total");
            return NETMANAGER_SUCCESS;
        }
    }
    NETNATIVE_LOGI("GetNetworkSharingTraffic failed");
    return NETMANAGER_ERROR;
}

static bool ConvertStrToLong(const std::string &str, int64_t &value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

void SharingManager::GetTraffic(std::smatch &matches, std::string &ifaceName, NetworkSharingTraffic &traffic,
    bool &isFindTx, bool &isFindRx)
{
    for (uint32_t i = 0; i < matches.size() - 1; i++) {
        std::string matchTemp = matches[i];
        NETNATIVE_LOG_D("GetNetworkCellularSharingTraffic matche[%{public}s]", matchTemp.c_str());
        std::string matchNext = matches[i + NEXT_LIST_CORRECT_DATA];
        if (matchTemp.find(CELLULAR_IFACE_NAME) != std::string::npos
            && matchNext.find(WLAN_IFACE_NAME) != std::string::npos && ((i - TWO_LIST_CORRECT_DATA) >= 0)) {
            int64_t send = 0;
            if (!ConvertStrToLong(matches[i - TWO_LIST_CORRECT_DATA].str(), send)) {
                return;
            }
            isFindTx = true;
            traffic.send = send;
            traffic.all += send;
            ifaceName = matchTemp;
        } else if (matchTemp.find(WLAN_IFACE_NAME) != std::string::npos
            && matchNext.find(CELLULAR_IFACE_NAME) != std::string::npos && ((i - TWO_LIST_CORRECT_DATA) >= 0)) {
            int64_t receive = 0;
            if (!ConvertStrToLong(matches[i - TWO_LIST_CORRECT_DATA].str(), receive)) {
                return;
            }
            isFindRx = true;
            traffic.receive = receive;
            traffic.all += receive;
        } else if (matchTemp.find(WLAN_IFACE_NAME) != std::string::npos
            && matchNext.find(WLAN_IFACE_NAME) != std::string::npos && ((i - TWO_LIST_CORRECT_DATA) >= 0)
            && ifaceName == "") {
            int64_t send = 0;
            if (!ConvertStrToLong(matches[i - TWO_LIST_CORRECT_DATA].str(), send)) {
                return;
            }
            isFindTx = true;
            traffic.send = send;
            traffic.all += send;
            ifaceName = matchTemp;
        } else if (matchTemp.find(WLAN_IFACE_NAME) != std::string::npos
            && matchNext.find(WLAN_IFACE_NAME) != std::string::npos && ((i - TWO_LIST_CORRECT_DATA) >= 0)
            && ifaceName.find(WLAN_IFACE_NAME) != std::string::npos) {
            int64_t receive = 0;
            if (!ConvertStrToLong(matches[i - TWO_LIST_CORRECT_DATA].str(), receive)) {
                return;
            }
            isFindRx = true;
            traffic.receive = receive;
            traffic.all += receive;
        }
    }
}

void SharingManager::CheckInited()
{
    std::lock_guard<std::mutex> guard(initedMutex_);
    if (inited_) {
        return;
    }
    InitChildChains();
}

void SharingManager::SetForwardRules(bool set, const std::string &cmds, std::string &cmdSet)
{
    const std::string op = set ? "-A" : "-D";
    cmdSet.append(op + cmds + "\n");
}

void SharingManager::CombineRestoreRules(const std::string &cmds, std::string &cmdSet)
{
    cmdSet.append(cmds + "\n");
}

int32_t SharingManager::EnableShareUnreachableRoute(RouteManager::TableType tableType)
{
    NETNATIVE_LOGI("EnableShareUnreachableRoute enter");
    bool routeRepeat = false;

    NetworkRouteInfo networkRouteInfo;
    networkRouteInfo.destination = "0.0.0.0/0";
    networkRouteInfo.nextHop = "unreachable";

    int32_t ret = RouteManager::AddRoute(tableType, networkRouteInfo, routeRepeat);
    if (ret != 0 && !routeRepeat) {
        NETNATIVE_LOGE("EnableShareUnreachableRoute fail, ret = %{public}d", ret);
    }

    NetworkRouteInfo networkRouteInfoV6;
    networkRouteInfoV6.destination = "::/0";
    networkRouteInfoV6.nextHop = "unreachable";

    ret = RouteManager::AddRoute(tableType, networkRouteInfoV6, routeRepeat);
    if (ret != 0 && !routeRepeat) {
        NETNATIVE_LOGE("EnableShareUnreachableRoute v6 fail, ret = %{public}d", ret);
    }
    return ret;
}

int32_t SharingManager::DisableShareUnreachableRoute(RouteManager::TableType tableType)
{
    NETNATIVE_LOGI("DisableShareUnreachableRoute enter");
    std::string interfaceName = "";
    std::string destinationName = "0.0.0.0/0";
    std::string nextHop = "unreachable";

    int32_t ret = RouteManager::RemoveRoute(tableType, interfaceName, destinationName, nextHop);
    if (ret != 0) {
        NETNATIVE_LOGE("DisableShareUnreachableRoute fail, ret = %{public}d", ret);
    }
    
    std::string destinationV6Name = "::/0";
    ret = RouteManager::RemoveRoute(tableType, interfaceName, destinationV6Name, nextHop);
    if (ret != 0) {
        NETNATIVE_LOGE("DisableShareUnreachableRoute V6 fail, ret = %{public}d", ret);
    }
    return ret;
}

void SharingManager::ClearForbidIpRules()
{
    std::lock_guard<std::mutex> guard(forbidIpMutex_);
    for (auto ip : forbidIpsMap_) {
        RouteManager::SetSharingUnreachableIpRule(RTM_DELRULE, wifiShareInterface_, ip.first, ip.second);
    }
    forbidIpsMap_.clear();
}

int32_t SharingManager::SetInternetAccessByIpForWifiShare(
    const std::string &ipAddr, uint8_t family, bool accessInternet, const std::string &clientNetIfName)
{
    if (wifiShareInterface_ == "") {
        NETNATIVE_LOGE("wifi share network not enable");
        return -1;
    }
    std::lock_guard<std::mutex> guard(forbidIpMutex_);
    if (accessInternet && forbidIpsMap_.find(ipAddr) != forbidIpsMap_.end()) {
        forbidIpsMap_.erase(ipAddr);
    } else if (!accessInternet && forbidIpsMap_.find(ipAddr) == forbidIpsMap_.end()) {
        forbidIpsMap_[ipAddr] = family;
    } else {
        NETNATIVE_LOGE("ip[%{public}s] set conflict, access[%{public}d]",
            NetManagerStandard::CommonUtils::ToAnonymousIp(ipAddr, true).c_str(), accessInternet);
        return -1;
    }
    uint16_t action = accessInternet ? RTM_DELRULE : RTM_NEWRULE;
    int32_t res = RouteManager::SetSharingUnreachableIpRule(action, wifiShareInterface_, ipAddr, family);
    return res;
}
} // namespace nmd
} // namespace OHOS
