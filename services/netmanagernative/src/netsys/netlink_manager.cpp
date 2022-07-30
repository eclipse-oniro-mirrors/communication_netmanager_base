/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "netlink_manager.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <sys/socket.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "netlink_native_listener.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
int32_t NetlinkManager::pid_ = 0;
using namespace NetlinkDefine;

NetlinkManager::NetlinkManager(int32_t pid)
{
    netlinkCallbacks_ = std::make_shared<std::vector<sptr<NetsysNative::INotifyCallback>>>();
    this->pid_ = pid;
}

NetlinkManager::~NetlinkManager()
{
    netlinkCallbacks_->clear();
    netlinkCallbacks_ = nullptr;
}

int32_t NetlinkManager::RegisterNetlinkCallback(sptr<NetsysNative::INotifyCallback> callback)
{
    if (callback == nullptr) {
        NETNATIVE_LOGI("callback is nullptr");
        return NetlinkResult::ERR_NULL_PTR;
    }
    for (auto &cb : *netlinkCallbacks_) {
        if (cb == callback) {
            NETNATIVE_LOGI("callback is already registered");
            return NetlinkResult::OK;
        }
    }
    netlinkCallbacks_->push_back(callback);
    NETNATIVE_LOGI("callback is registered successfully");
    return NetlinkResult::OK;
}

int32_t NetlinkManager::UnRegisterNetlinkCallback(sptr<NetsysNative::INotifyCallback> callback)
{
    if (callback == nullptr) {
        NETNATIVE_LOGI("callback is nullptr");
        return NetlinkResult::ERR_NULL_PTR;
    }
    for (auto it = netlinkCallbacks_->begin(); it != netlinkCallbacks_->end();) {
        if (*it == callback) {
            it = netlinkCallbacks_->erase(it);
            NETNATIVE_LOGI("callback is unregistered successfully");
            return NetlinkResult::OK;
        } else {
            ++it;
        }
    }
    NETNATIVE_LOGI("callback is not registered");
    return NetlinkResult::ERR_INVALID_PARAM;
}

std::unique_ptr<NetlinkProcessor> NetlinkManager::SetSocket(int32_t &sock, int32_t netlinkType, uint32_t groups,
                                                            int32_t format, bool configNflog)
{
    sockaddr_nl nladdr;
    int32_t size = BUFFER_SIZE;
    int32_t on = 1;

    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_pid = 0;
    nladdr.nl_groups = groups;

    if ((sock = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, netlinkType)) < 0) {
        NETNATIVE_LOGE("Unable to create netlink socket for family %{public}d: %{public}s = %{public}d", netlinkType,
                       strerror(errno), errno);
        return nullptr;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &size, sizeof(size)) < 0 &&
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0) {
        NETNATIVE_LOGE("Unable to set socket receive buffer size error : %{public}d, EMSG: %{public}s", errno,
                       strerror(errno));
        close(sock);
        return nullptr;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
        NETNATIVE_LOGE("Unable to set uevent socket SO_PASSCRED option: %{public}d, EMSG: %{public}s", errno,
                       strerror(errno));
        close(sock);
        return nullptr;
    }

    if (bind(sock, reinterpret_cast<sockaddr *>(&nladdr), sizeof(nladdr)) < 0) {
        NETNATIVE_LOGE("Unable to bind netlink socket: %{public}d, EMSG: %{public}s", errno, strerror(errno));
        close(sock);
        return nullptr;
    }

    std::unique_ptr<NetlinkProcessor> processor = std::make_unique<NetlinkProcessor>(netlinkCallbacks_, sock, format);
    if (processor->Start()) {
        NETNATIVE_LOGE("Unable to start netlink handler: %{public}d, EMSG: %{public}s", errno, strerror(errno));
        close(sock);
        return nullptr;
    }

    return processor;
}

int32_t NetlinkManager::StartListener()
{
    if ((ueventProc_ = SetSocket(ueventSocket_, NETLINK_KOBJECT_UEVENT, UEVENT_GROUP, NETLINK_FORMAT_ASCII, false)) ==
        nullptr) {
        NETNATIVE_LOGE("Unable to open uevent socket");
        return NetlinkResult::ERROR;
    }
    if ((routeProc_ = SetSocket(routeSocket_, NETLINK_ROUTE,
                                RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE |
                                    RTMGRP_IPV6_ROUTE | (1 << (RTNLGRP_ND_USEROPT - 1)),
                                NETLINK_FORMAT_BINARY, false)) == nullptr) {
        NETNATIVE_LOGE("Unable to open route socket");
        return NetlinkResult::ERROR;
    }
    if ((quotaProc_ = SetSocket(quotaSocket_, NETLINK_NFLOG, NFLOG_QUOTA_GROUP, NETLINK_FORMAT_BINARY, false)) ==
        nullptr) {
        NETNATIVE_LOGE("Unable to open qlog quota socket, check if xt_quota2 can send via UeventHandler");
    }

    if ((strictProc_ = SetSocket(strictSocket_, NETLINK_NETFILTER, 0, NETLINK_FORMAT_BINARY_UNICAST, true)) ==
        nullptr) {
        NETNATIVE_LOGE("Unable to open strict socket, check if xt_strict can send via UeventHandler");
    }
    return NetlinkResult::OK;
}

int32_t NetlinkManager::StopListener()
{
    int32_t status = NetlinkResult::OK;

    if (ueventProc_->Stop()) {
        status = NetlinkResult::ERROR;
    }
    ueventProc_ = nullptr;
    close(ueventSocket_);
    ueventSocket_ = -1;

    if (routeProc_->Stop()) {
        status = NetlinkResult::ERROR;
    }
    routeProc_ = nullptr;
    close(routeSocket_);
    routeSocket_ = -1;

    if (quotaProc_ != nullptr) {
        if (quotaProc_->Stop()) {
            status = NetlinkResult::ERROR;
        }
        quotaProc_ = nullptr;
        close(quotaSocket_);
        quotaSocket_ = -1;
    }

    if (strictProc_ != nullptr) {
        if (strictProc_->Stop()) {
            status = NetlinkResult::ERROR;
        }
        strictProc_ = nullptr;
        close(strictSocket_);
        strictSocket_ = -1;
    }

    return status;
}
} // namespace nmd
} // namespace OHOS
