/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <unistd.h>

#ifndef SHARING_MANAGER_DEPS
#include "netnative_log_wrapper.h"
#include "net_manager_constants.h"
#include "route_manager.h"
#endif

namespace OHOS {
namespace nmd {
#ifndef SHARING_MANAGER_DEPS
using namespace NetManagerStandard;
#endif
namespace {
constexpr const char *IPV4_FORWARDING_PROC_FILE = "/proc/sys/net/ipv4/ip_forward";
constexpr const char *IPV6_FORWARDING_PROC_FILE = "/proc/sys/net/ipv6/conf/all/forwarding";

// commands of create tables
const std::string CREATE_TETHERCTRL_NAT_POSTROUTING = "-t nat -N tetherctrl_nat_POSTROUTING";
const std::string CREATE_TETHERCTRL_FORWARD = "-t filter -N tetherctrl_FORWARD";
const std::string CREATE_TETHERCTRL_COUNTERS = "-t filter -N tetherctrl_counters";
const std::string CREATE_TETHERCTRL_MANGLE_FORWARD = "-t mangle -N tetherctrl_mangle_FORWARD";

// commands of set nat
const std::string APPEND_NAT_POSTROUTING = "-t nat -A POSTROUTING -j tetherctrl_nat_POSTROUTING";
const std::string APPEND_MANGLE_FORWARD = "-t mangle -A FORWARD -j tetherctrl_mangle_FORWARD";
const std::string APPEND_TETHERCTRL_MANGLE_FORWARD =
    "-t mangle -A tetherctrl_mangle_FORWARD "
    "-p tcp -m tcp --tcp-flags SYN SYN -j TCPMSS --clamp-mss-to-pmtu";
const std::string CLEAR_TETHERCTRL_NAT_POSTROUTING = "-t nat -F tetherctrl_nat_POSTROUTING";
const std::string CLEAR_TETHERCTRL_MANGLE_FORWARD = "-t mangle -F tetherctrl_mangle_FORWARD";
const std::string DELETE_TETHERCTRL_NAT_POSTROUTING = "-t nat -D POSTROUTING -j tetherctrl_nat_POSTROUTING";
const std::string DELETE_TETHERCTRL_MANGLE_FORWARD = "-t mangle -D FORWARD -j tetherctrl_mangle_FORWARD";

#ifndef SHARING_MANAGER_DEPS
const std::string ENABLE_NAT(const std::string &down)
{
    return "-t nat -A tetherctrl_nat_POSTROUTING -o " + down + " -j MASQUERADE";
}
#endif

// commands of set ipfwd, all commands with filter
const std::string FORWARD_JUMP_TETHERCTRL_FORWARD = " FORWARD -j tetherctrl_FORWARD";
const std::string SET_TETHERCTRL_FORWARD_DROP = " tetherctrl_FORWARD -j DROP";
const std::string SET_TETHERCTRL_FORWARD1(const std::string &from, const std::string &to)
{
    return " tetherctrl_FORWARD -i " + from + " -o " + to + " -m state --state RELATED,ESTABLISHED"
        " -g tetherctrl_counters";
}

const std::string SET_TETHERCTRL_FORWARD2(const std::string &from, const std::string &to)
{
    return " tetherctrl_FORWARD -i " + to + " -o " + from + " -m state --state INVALID -j DROP";
}

const std::string SET_TETHERCTRL_FORWARD3(const std::string &from, const std::string &to)
{
    return " tetherctrl_FORWARD -i " + to + " -o " + from + " -g tetherctrl_counters";
}

const std::string SET_TETHERCTRL_COUNTERS1(const std::string &from, const std::string &to)
{
    return " tetherctrl_counters -i " + to + " -o " + from + " -j RETURN";
}

const std::string SET_TETHERCTRL_COUNTERS2(const std::string &from, const std::string &to)
{
    return " tetherctrl_counters -i " + from + " -o " + to + " -j RETURN";
}

bool WriteToFile(const char *filename, const char *value)
{
    if (filename == nullptr) {
        return false;
    }
    int fd = open(filename, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
#ifndef SHARING_MANAGER_DEPS
        NETNATIVE_LOGE("failed to open %{public}s: %{public}s", filename, strerror(errno));
#endif
        return false;
    }

    const ssize_t len = strlen(value);
    if (write(fd, value, len) != len) {
#ifndef SHARING_MANAGER_DEPS
        NETNATIVE_LOGE("faield to write %{public}s to %{public}s: %{public}s", value, filename, strerror(errno));
#endif
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

void Rollback()
{
#ifndef SHARING_MANAGER_DEPS
    NETNATIVE_LOGE("rollback");
#endif
    system("iptables-restore -filter < /tmp/ipfwd.bak");
}
} // namespace

SharingManager::SharingManager()
{
#ifndef SHARING_MANAGER_DEPS
    iptablesWrapper_ = DelayedSingleton<IptablesWrapper>::GetInstance();
#endif
}

void SharingManager::InitChildChains()
{
#ifndef SHARING_MANAGER_DEPS
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, CREATE_TETHERCTRL_NAT_POSTROUTING);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, CREATE_TETHERCTRL_FORWARD);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, CREATE_TETHERCTRL_COUNTERS);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, CREATE_TETHERCTRL_MANGLE_FORWARD);
#endif
    inited_ = true;
}

int32_t SharingManager::IpEnableForwarding(const std::string &requestor)
{
#ifndef SHARING_MANAGER_DEPS
    NETNATIVE_LOGE("IpEnableForwarding requestor: %{public}s", requestor.c_str());
#endif
    forwardingRequests_.insert(requestor);
    return SetIpFwdEnable();
}

int32_t SharingManager::IpDisableForwarding(const std::string &requestor)
{
#ifndef SHARING_MANAGER_DEPS
    NETNATIVE_LOGE("IpDisableForwarding requestor: %{public}s", requestor.c_str());
#endif
    forwardingRequests_.erase(requestor);
    return SetIpFwdEnable();
}

int32_t SharingManager::EnableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    CheckInited();
    if (downstreamIface == upstreamIface) {
#ifndef SHARING_MANAGER_DEPS
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %{public}s",
            downstreamIface.c_str(), upstreamIface.c_str());
#endif
        return -1;
    }
    int32_t result = 0;
#ifndef SHARING_MANAGER_DEPS
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, APPEND_NAT_POSTROUTING);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, APPEND_MANGLE_FORWARD);

    NETNATIVE_LOGE("EnableNat downstreamIface: %{public}s, upstreamIface: %{public}s",
        downstreamIface.c_str(), upstreamIface.c_str());

    result = iptablesWrapper_->RunCommand(IPTYPE_IPV4, ENABLE_NAT(downstreamIface));
    if (result) {
        return result;
    }

    result = iptablesWrapper_->RunCommand(IPTYPE_IPV4, APPEND_TETHERCTRL_MANGLE_FORWARD);
    if (result) {
        return result;
    }
#endif

    return result;
}

int32_t SharingManager::DisableNat(const std::string &downstreamIface, const std::string &upstreamIface)
{
    CheckInited();
    if (downstreamIface == upstreamIface) {
#ifndef SHARING_MANAGER_DEPS
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %s", downstreamIface.c_str(), upstreamIface.c_str());
#endif
        return -1;
    }
    int32_t result = 0;

#ifndef SHARING_MANAGER_DEPS
    NETNATIVE_LOGE("DisableNat downstreamIface: %{public}s, upstreamIface: %{public}s", downstreamIface.c_str(),
        upstreamIface.c_str());

    result = iptablesWrapper_->RunCommand(IPTYPE_IPV4, CLEAR_TETHERCTRL_NAT_POSTROUTING);
    if (result) {
        return result;
    }
    result = iptablesWrapper_->RunCommand(IPTYPE_IPV4, CLEAR_TETHERCTRL_MANGLE_FORWARD);

    iptablesWrapper_->RunCommand(IPTYPE_IPV4, DELETE_TETHERCTRL_NAT_POSTROUTING);
    iptablesWrapper_->RunCommand(IPTYPE_IPV4, DELETE_TETHERCTRL_MANGLE_FORWARD);
#endif
    return result;
}

int32_t SharingManager::SetIpFwdEnable()
{
    bool success = true;
    bool disable = forwardingRequests_.empty();
    const char *value = disable ? "0" : "1";
    success &= WriteToFile(IPV4_FORWARDING_PROC_FILE, value);
    success &= WriteToFile(IPV6_FORWARDING_PROC_FILE, value);
    if (success) {
        return -1;
    }
    return 0;
}

int32_t SharingManager::IpfwdAddInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    CheckInited();
    if (fromIface == toIface) {
#ifndef SHARING_MANAGER_DEPS
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %{public}s", fromIface.c_str(), toIface.c_str());
#endif
        return -1;
    }
#ifndef SHARING_MANAGER_DEPS
    NETNATIVE_LOGE("IpfwdAddInterfaceForward fromIface: %{public}s, toIface: %{public}s",
        fromIface.c_str(), toIface.c_str());
#endif

    if (interfaceForwards_.empty()) {
        SetForwardRules(true, FORWARD_JUMP_TETHERCTRL_FORWARD);
    }

    int32_t result = 0;

    system("iptables-save -t filter > /tmp/ipfwd.bak");

    /*
     * Add a forward rule, when the status of packets is RELATED,
     * ESTABLISED and from fromIface to toIface, goto tetherctrl_counters
     */
    result = SetForwardRules(true, SET_TETHERCTRL_FORWARD1(fromIface, toIface));
    if (result) {
        return result;
    }

    /*
     * Add a forward rule, when the status is INVALID and from toIface to fromIface, just drop
     */
    result = SetForwardRules(true, SET_TETHERCTRL_FORWARD2(fromIface, toIface));
    if (result) {
        Rollback();
        return result;
    }

    /*
     * Add a forward rule, from toIface to fromIface, goto tetherctrl_counters
     */
    result = SetForwardRules(true, SET_TETHERCTRL_FORWARD3(fromIface, toIface));
    if (result) {
        Rollback();
        return result;
    }

    /*
     * Add a forward rule, drop others
     */
    result = SetForwardRules(true, SET_TETHERCTRL_FORWARD_DROP);
    if (result) {
        Rollback();
        return result;
    }

    /*
     * Add a forward rule, if from toIface to fromIface return chain of father
     */
    result = SetForwardRules(true, SET_TETHERCTRL_COUNTERS1(fromIface, toIface));
    if (result) {
        Rollback();
        return result;
    }

    /*
     * Add a forward rule, if from fromIface to toIface return chain of father
     */
    result = SetForwardRules(true, SET_TETHERCTRL_COUNTERS2(fromIface, toIface));
    if (result) {
        Rollback();
        return result;
    }
    interfaceForwards_.insert(fromIface + toIface);

    return 0;
}

int32_t SharingManager::IpfwdRemoveInterfaceForward(const std::string &fromIface, const std::string &toIface)
{
    CheckInited();
    if (fromIface == toIface) {
#ifndef SHARING_MANAGER_DEPS
        NETNATIVE_LOGE("Duplicate interface specified: %{public}s %{public}s", fromIface.c_str(), toIface.c_str());
#endif
        return -1;
    }

#ifndef SHARING_MANAGER_DEPS
    NETNATIVE_LOGE("IpfwdRemoveInterfaceForward fromIface: %{public}s, toIface: %{public}s",
        fromIface.c_str(), toIface.c_str());
#endif

#ifndef SHARING_MANAGER_DEPS
    SetForwardRules(false, SET_TETHERCTRL_FORWARD1(fromIface, toIface));
    SetForwardRules(false, SET_TETHERCTRL_FORWARD2(fromIface, toIface));
    SetForwardRules(false, SET_TETHERCTRL_FORWARD2(fromIface, toIface));
    SetForwardRules(false, SET_TETHERCTRL_FORWARD_DROP);
    SetForwardRules(false, SET_TETHERCTRL_COUNTERS1(fromIface, toIface));
    SetForwardRules(false, SET_TETHERCTRL_COUNTERS2(fromIface, toIface));

    interfaceForwards_.erase(fromIface + toIface);
    if (interfaceForwards_.empty()) {
        SetForwardRules(false, FORWARD_JUMP_TETHERCTRL_FORWARD);
    }
#endif

    return 0;
}

void SharingManager::CheckInited()
{
    std::lock_guard<std::mutex> guard(initedMutex_);
    if (inited_) {
        return;
    }
    InitChildChains();
}

int32_t SharingManager::SetForwardRules(bool set, const std::string &cmds)
{
    const std::string op = set ? "-A" : "-D";
#ifndef SHARING_MANAGER_DEPS
    return iptablesWrapper_->RunCommand(IPTYPE_IPV4, "-t filter " + op + cmds);
#endif
    return 0;
}
} // namespace nmd
} // namespace OHOS
