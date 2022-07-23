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

#include "netlink_processor.h"

#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <climits>
#include <memory>
#include <cstdarg>
#include <cstdlib>

#include "netlink_manager.h"
#include "netlink_message_decoder.h"
#include "netlink_define.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"

using namespace OHOS::NetManagerStandard::CommonUtils;
namespace OHOS {
namespace nmd {
using namespace NetlinkDefine;
NetlinkProcessor::NetlinkProcessor(std::shared_ptr<std::vector<sptr<NetsysNative::INotifyCallback>>> callback,
                                   int32_t listenerSocket, int32_t format)
    : NetlinkNativeListener(listenerSocket, false, format)
{
    NETNATIVE_LOGI("NetlinkProcessor: Create NetlinkProcessor");
    netlinkCallbacks_ = callback;
}

int32_t NetlinkProcessor::Start()
{
    return this->OpenMonitor();
}

int32_t NetlinkProcessor::Stop()
{
    return this->CloseMonitor();
}

static int64_t ParseIfIndex(const std::string &ifIndex)
{
    if (ifIndex.empty()) {
        return NetlinkResult::OK;
    }
    int64_t ifaceIndex = std::strtol(ifIndex.c_str(), nullptr, DECIMALISM);
    if (errno == ERANGE && (ifaceIndex == LONG_MAX || ifaceIndex == LONG_MIN)) {
        return NetlinkResult::OK;
    }
    return ifaceIndex;
}

void NetlinkProcessor::OnEvent(std::shared_ptr<NetlinkMessageDecoder> message)
{
    if (message == nullptr) {
        NETNATIVE_LOGI("NetlinkProcessor: OnEvent: message is nullptr");
        return;
    }
    OnStateChange(message);
}

void NetlinkProcessor::OnStateChange(const std::shared_ptr<NetlinkMessageDecoder> message)
{
    std::thread t([this, message]() {
        const std::string &subsys = message->GetSubsystem();
        if (subsys.empty()) {
            NETNATIVE_LOGW("No subsystem found in netlink event");
            return;
        }
        if (subsys == "net") {
            HandleSubSysNet(message);
        } else if (subsys == "route") {
        } else if ((subsys == "qlog") || (subsys == "xt_quota2")) {
            HandleSubSysQlog(message);
        } else if ((subsys == "strict")) {
            HandleSubSysStrict(message);
        } else if ((subsys == "xtIdletimer")) {
            HandleSubSysIdLetimer(message);
        }
    });
    t.detach();
}

void NetlinkProcessor::HandleSubSysNet(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    NetlinkMessageDecoder::Action action = message->GetAction();
    const std::string &iface = message->FindParam("INTERFACE");
    if ((action == NetlinkMessageDecoder::Action::ADD) || (action == NetlinkMessageDecoder::Action::LINKUP) ||
        (action == NetlinkMessageDecoder::Action::LINKDOWN)) {
        const std::string &ifIndex = message->FindParam("IFINDEX");
        int64_t ifaceIndex = ParseIfIndex(ifIndex);
        if (ifaceIndex) {
            // T.B.D.
        } else {
            NETNATIVE_LOGE("invalid interface index: %{public}s(%{public}s)", iface.c_str(), ifIndex.c_str());
        }
    }
    switch (action) {
        case NetlinkMessageDecoder::Action::ADD:
            OnInterfaceAdd(iface);
            break;
        case NetlinkMessageDecoder::Action::REMOVE:
            OnInterfaceRemove(iface);
            break;
        case NetlinkMessageDecoder::Action::CHANGE:
            message->Dump();
            OnInterfaceChange(iface, true);
            break;
        case NetlinkMessageDecoder::Action::LINKUP:
            OnInterfaceLinkStateChange(iface, true);
            break;
        case NetlinkMessageDecoder::Action::LINKDOWN:
            OnInterfaceLinkStateChange(iface, false);
            break;
        case NetlinkMessageDecoder::Action::ADDRESSUPDATE:
        case NetlinkMessageDecoder::Action::ADDRESSREMOVED:
            HandleAddressChange(message);
            break;
        case NetlinkMessageDecoder::Action::RDNSS:
            HandleRndssChange(message);
            break;
        case NetlinkMessageDecoder::Action::ROUTEUPDATED:
        case NetlinkMessageDecoder::Action::ROUTEREMOVED:
            HandleRouteChange(message);
            break;
        default:
            break;
    }
}

void NetlinkProcessor::HandleAddressChange(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    const std::string &iface = message->FindParam("INTERFACE");
    NetlinkMessageDecoder::Action action = message->GetAction();

    const std::string &address = message->FindParam("ADDRESS");
    const std::string &flags = message->FindParam("FLAGS");
    const std::string &scope = message->FindParam("SCOPE");
    const std::string &ifIndex = message->FindParam("IFINDEX");
    char addrstr[INET6_ADDRSTRLEN + strlen("/128")];
    strlcpy(addrstr, address.c_str(), sizeof(addrstr));
    char *slash = strchr(addrstr, '/');
    if (slash) {
        *slash = '\0';
    }

    int64_t ifaceIndex = ParseIfIndex(ifIndex);
    if (!ifaceIndex) {
        NETNATIVE_LOGE("invalid interface index: %{public}s(%{public}s)", iface.c_str(), ifIndex.c_str());
    }
    const bool addrUpdated = (action == NetlinkMessageDecoder::Action::ADDRESSUPDATE);

    if (!iface.empty() && iface[0] && !address.empty() && !flags.empty() && !scope.empty()) {
        if (addrUpdated) {
            OnInterfaceAddressUpdate(address, iface, ConvertToInt64(flags), ConvertToInt64(scope));
        } else {
            OnInterfaceAddressRemove(address, iface, ConvertToInt64(flags), ConvertToInt64(scope));
        }
    }
}

void NetlinkProcessor::HandleRouteChange(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    NetlinkMessageDecoder::Action action = message->GetAction();
    const std::string &route = message->FindParam("ROUTE");
    const std::string &gateway = message->FindParam("GATEWAY");
    const std::string &iface = message->FindParam("INTERFACE");
    if (!route.empty() && (!gateway.empty() || !iface.empty())) {
        OnRouteChange((action == NetlinkMessageDecoder::Action::ROUTEUPDATED) ? true : false, route,
                      gateway.empty() ? "" : gateway, (iface.empty() ? "" : iface));
    }
}

void NetlinkProcessor::HandleSubSysIdLetimer(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    const std::string &label = message->FindParam("INTERFACE");
    const std::string &state = message->FindParam("STATE");
    const std::string &timestamp = message->FindParam("TIME_NS");
    const std::string &uid = message->FindParam("UID");
    if (!state.empty()) {
        bool isActive = !(state == "active");
        int64_t processTimestamp = timestamp.empty() ? 0 : std::strtoll(timestamp.c_str(), nullptr, DECIMALISM);
        int32_t intLabel;
        if (ParseInt(label, &intLabel)) {
            const int32_t reportedUid = (!uid.empty() && isActive) ? ConvertToInt64(uid) : -1;
            OnInterfaceClassActivityChange(intLabel, isActive, processTimestamp, reportedUid);
        }
    }
}

void NetlinkProcessor::HandleSubSysQlog(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    const std::string &alertName = message->FindParam("ALERT_NAME");
    const std::string &iface = message->FindParam("INTERFACE");
    if ((!alertName.empty()) && (!iface.empty())) {
        OnQuotaLimitReache(alertName, iface);
    }
}

void NetlinkProcessor::HandleRndssChange(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    const std::string &iface = message->FindParam("INTERFACE");
    const std::string &lifetime = message->FindParam("LIFETIME");
    const std::string &servers = message->FindParam("SERVERS");
    if (!lifetime.empty() && !servers.empty()) {
        OnInterfaceDnsServersUpdate(iface, ConvertToInt64(lifetime), Split(servers, ","));
    }
}

void NetlinkProcessor::HandleSubSysStrict(const std::shared_ptr<NetlinkMessageDecoder> &message)
{
    const std::string &uid = message->FindParam("UID");
    const std::string &hex = message->FindParam("HEX");
    if (!uid.empty() && !hex.empty()) {
        OnStrictCleartext(ConvertToInt64(uid), hex);
    }
}

void NetlinkProcessor::OnInterfaceAdd(const std::string &ifName)
{
    NETNATIVE_LOGI("interface added: %{public}s", ifName.c_str());
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnInterfaceAdded(ifName);
    }
}

void NetlinkProcessor::OnInterfaceRemove(const std::string &ifName)
{
    NETNATIVE_LOGI("interface removed: %{public}s", ifName.c_str());
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnInterfaceRemoved(ifName);
    }
}

void NetlinkProcessor::OnInterfaceChange(const std::string &ifName, bool up)
{
    NETNATIVE_LOGI("interface Change: %{public}s", ifName.c_str());
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnInterfaceChanged(ifName, up);
    }
}

void NetlinkProcessor::OnInterfaceLinkStateChange(const std::string &ifName, bool up)
{
    NETNATIVE_LOGI("interface link state Change: %{public}s", ifName.c_str());
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnInterfaceLinkStateChanged(ifName, up);
    }
}

void NetlinkProcessor::OnQuotaLimitReache(const std::string &labelName, const std::string &ifName)
{
    NETNATIVE_LOGI("OnQuotaLimitReache: %{public}s, %{public}s", labelName.c_str(), ifName.c_str());
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnBandwidthReachedLimit(labelName, ifName);
    }
}

void NetlinkProcessor::OnInterfaceClassActivityChange(int32_t label, bool isActive, int64_t timestamp, int32_t uid)
{
    NETNATIVE_LOGI("OnInterfaceClassActivityChange: %{public}d, %{public}d, %{public}d", label, isActive, uid);
}

void NetlinkProcessor::OnInterfaceAddressUpdate(const std::string &addr, const std::string &ifName, int32_t flags,
                                                int32_t scope)
{
    NETNATIVE_LOGI("OnInterfaceAddressUpdated: %{public}s, %{public}s, %{public}d, %{public}d",
                   ToAnonymousIp(addr).c_str(), ifName.c_str(), flags, scope);
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    }
}

void NetlinkProcessor::OnInterfaceAddressRemove(const std::string &addr, const std::string &ifName, int32_t flags,
                                                int32_t scope)
{
    NETNATIVE_LOGI("OnInterfaceAddressRemove: %{public}s, %{public}s, %{public}d, %{public}d",
                   ToAnonymousIp(addr).c_str(), ifName.c_str(), flags, scope);
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    }
}

void NetlinkProcessor::OnInterfaceDnsServersUpdate(const std::string &ifName, int64_t lifetime,
                                                   const std::vector<std::string> &servers)
{
    NETNATIVE_LOGI("NotifyInterfaceDnsServers: %{public}s, %{public}s", ifName.c_str(), servers.data()->c_str());
}

void NetlinkProcessor::OnRouteChange(bool updated, const std::string &route, const std::string &gateway,
                                     const std::string &ifName)
{
    NETNATIVE_LOGI("OnRouteChange: %{public}s, %{public}s, %{public}s, %{public}s", updated ? "updated" : "removed",
                   ToAnonymousIp(route).c_str(), ToAnonymousIp(gateway).c_str(), ifName.c_str());
    for (auto &callback : *netlinkCallbacks_) {
        callback->OnRouteChanged(updated, route, gateway, ifName);
    }
}

void NetlinkProcessor::OnStrictCleartext(uid_t uid, const std::string &hex)
{
    NETNATIVE_LOGI("NotifyStrictCleartext: %{public}d, %{public}s", uid, hex.c_str());
}
} // namespace nmd
} // namespace OHOS
