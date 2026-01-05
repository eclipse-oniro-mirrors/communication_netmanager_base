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

#include "netlink_manager.h"

#include <cerrno>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>

#include "netlink_define.h"
#include "netnative_log_wrapper.h"
#include "wrapper_distributor.h"
#ifdef FEATURE_NET_FIREWALL_ENABLE
#include "securec.h"
#endif

namespace OHOS {
namespace nmd {
using namespace NetlinkDefine;
namespace {
constexpr int32_t NFLOG_QUOTA_GROUP = 1;
constexpr int32_t UEVENT_GROUP = 0xffffffff;
constexpr uint8_t NFLOG_NETFILTER_GROUP_ID = 0;
#ifdef FEATURE_NET_FIREWALL_ENABLE
constexpr uint32_t PACKET_COPY_LENGTH = 256;
constexpr int32_t LOCAL_NFLOG_CONFIG = NFNL_SUBSYS_ULOG << 8 | NFULNL_MSG_CONFIG;
constexpr int16_t MSG_BUFFER_SIZE = 512;

struct NflogConfigMsg {
    char buffer[MSG_BUFFER_SIZE]{};
    nlmsghdr *header{nullptr};
    size_t usedLength{0};
};
#endif

struct DistributorParam {
    int32_t groups;
    int32_t format;
    bool flag;
};

const std::map<int32_t, DistributorParam> distributorParamList_ = {
    {NETLINK_KOBJECT_UEVENT, {UEVENT_GROUP, NETLINK_FORMAT_ASCII, false}},
    {NETLINK_ROUTE,
     {RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE |
          (1 << (RTNLGRP_ND_USEROPT - 1)),
      NETLINK_FORMAT_BINARY, false}},
    {NETLINK_NFLOG, {NFLOG_QUOTA_GROUP, NETLINK_FORMAT_BINARY, false}},
    {NETLINK_NETFILTER, {NFLOG_NETFILTER_GROUP_ID, NETLINK_FORMAT_BINARY_UNICAST, true}}};

std::map<int32_t, std::unique_ptr<WrapperDistributor>> distributorMap_;

#ifdef FEATURE_NET_FIREWALL_ENABLE
bool InitNflogConfig(NflogConfigMsg &msg, uint16_t groupId)
{
    msg.header = reinterpret_cast<nlmsghdr *>(msg.buffer);
    msg.header->nlmsg_len = NLMSG_LENGTH(sizeof(nfgenmsg));
    msg.header->nlmsg_type = LOCAL_NFLOG_CONFIG;
    msg.header->nlmsg_flags = NLM_F_REQUEST;
    msg.header->nlmsg_seq = static_cast<uint32_t>(time(nullptr));
    msg.header->nlmsg_pid = 0;

    nfgenmsg *nfHeader = reinterpret_cast<nfgenmsg *>(NLMSG_DATA(msg.header));
    nfHeader->nfgen_family = AF_UNSPEC;
    nfHeader->version = NFNETLINK_V0;
    nfHeader->res_id = htons(groupId);

    msg.usedLength = NLMSG_ALIGN(msg.header->nlmsg_len);
    return true;
}

bool AddCmdAttr(NflogConfigMsg &msg, uint8_t command)
{
    nfulnl_msg_config_cmd cmd{};
    cmd.command = command;

    size_t need = NLA_HDRLEN + sizeof(cmd);
    size_t end  = msg.usedLength + NLA_ALIGN(need);
    if (end > sizeof(msg.buffer)) {
        return false;
    }
    auto *attr = reinterpret_cast<nlattr *>(msg.buffer + msg.usedLength);
    attr->nla_type = NFULA_CFG_CMD;
    attr->nla_len  = static_cast<uint16_t>(need);

    void *dest = reinterpret_cast<char *>(attr) + NLA_HDRLEN;
    size_t dstLen = need - NLA_HDRLEN;
    if (memcpy_s(dest, dstLen, &cmd, sizeof(cmd)) != EOK) {
        return false;
    }
    msg.usedLength = end;
    return true;
}

bool AddModeAttr(NflogConfigMsg &msg, uint8_t copyMode, uint32_t copyRange)
{
    nfulnl_msg_config_mode mode{};
    mode.copy_mode  = copyMode;
    mode.copy_range = htonl(copyRange);

    size_t need = NLA_HDRLEN + sizeof(mode);
    size_t end  = msg.usedLength + NLA_ALIGN(need);
    if (end > sizeof(msg.buffer)) {
        return false;
    }
    auto *attr = reinterpret_cast<nlattr *>(msg.buffer + msg.usedLength);
    attr->nla_type = NFULA_CFG_MODE;
    attr->nla_len  = static_cast<uint16_t>(need);

    void *dest = reinterpret_cast<char *>(attr) + NLA_HDRLEN;
    size_t dstLen = need - NLA_HDRLEN;
    if (memcpy_s(dest, dstLen, &mode, sizeof(mode)) != EOK) {
        return false;
    }
    msg.usedLength = end;
    return true;
}

bool SendNflogConfig(int32_t socketFd, uint16_t groupId, uint8_t copyMode, uint32_t copyRange)
{
    NflogConfigMsg msg;
    if (!InitNflogConfig(msg, groupId)) {
        return false;
    }
    if (!AddCmdAttr(msg, NFULNL_CFG_CMD_BIND)) {
        return false;
    }
    if (!AddModeAttr(msg, copyMode, copyRange)) {
        return false;
    }
    msg.header->nlmsg_len = msg.usedLength;
    return send(socketFd, msg.buffer, msg.header->nlmsg_len, 0) >= 0;
}

bool SendNflogUnbind(int32_t socketFd, uint16_t groupId)
{
    NflogConfigMsg msg;
    if (!InitNflogConfig(msg, groupId)) {
        return false;
    }
    if (!AddCmdAttr(msg, NFULNL_CFG_CMD_UNBIND)) {
        return false;
    }
    msg.header->nlmsg_len = msg.usedLength;
    return send(socketFd, msg.buffer, msg.header->nlmsg_len, 0) >= 0;
}
#endif

bool CreateNetlinkDistributor(int32_t netlinkType, const DistributorParam &param, std::mutex& externMutex)
{
    sockaddr_nl sockAddr;
    int32_t size = BUFFER_SIZE;
    int32_t on = 1;
    int32_t socketFd;

    sockAddr.nl_family = AF_NETLINK;
    sockAddr.nl_pid = 0;
    sockAddr.nl_groups = param.groups;

    if ((socketFd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, netlinkType)) < 0) {
        NETNATIVE_LOGE("Creat socket for family failed NetLinkType is %{public}d: %{public}s = %{public}d", netlinkType,
                       strerror(errno), errno);
        return false;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_RCVBUFFORCE, &size, sizeof(size)) < 0 &&
        setsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0) {
        NETNATIVE_LOGE("Set buffer for revieve msg failed the error is : %{public}d, EMSG: %{public}s", errno,
                       strerror(errno));
        close(socketFd);
        return false;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
        NETNATIVE_LOGE("Uevent socket SO_PASSCRED set failed dump for this: %{public}d, EMSG: %{public}s", errno,
                       strerror(errno));
        close(socketFd);
        return false;
    }

    if (bind(socketFd, reinterpret_cast<sockaddr *>(&sockAddr), sizeof(sockAddr)) < 0) {
        NETNATIVE_LOGE("Bind netlink socket failed dumps is this : %{public}d, EMSG: %{public}s", errno,
                       strerror(errno));
        close(socketFd);
        return false;
    }
#ifdef FEATURE_NET_FIREWALL_ENABLE
    uint16_t groupId = static_cast<uint16_t>(param.groups);
    if (netlinkType == NETLINK_NETFILTER &&
        !SendNflogConfig(socketFd, groupId, NFULNL_COPY_PACKET, PACKET_COPY_LENGTH)) {
        bool unbindResult = SendNflogUnbind(socketFd, groupId);
        NETNATIVE_LOGE("Configure NFLOG failed, unbindResult=%{public}d, group=%{public}d", unbindResult, groupId);
        close(socketFd);
        return false;
    }
#endif

    NETNATIVE_LOGI("CreateNetlinkDistributor netlinkType: %{public}d, socketFd: %{public}d", netlinkType, socketFd);
    distributorMap_[netlinkType] = std::make_unique<WrapperDistributor>(socketFd, param.format, externMutex);
    return true;
}
} // namespace

NetlinkManager::NetlinkManager()
{
    for (const auto &it : distributorParamList_) {
        CreateNetlinkDistributor(it.first, it.second, linkCallbackMutex_);
    }
    if (callbacks_ == nullptr) {
        callbacks_ = std::make_shared<std::vector<sptr<NetsysNative::INotifyCallback>>>();
    }
}

NetlinkManager::~NetlinkManager()
{
    if (callbacks_ != nullptr) {
        callbacks_->clear();
        callbacks_ = nullptr;
    }
}

int32_t NetlinkManager::StartListener()
{
    for (auto &it : distributorMap_) {
        if (it.second == nullptr) {
            continue;
        }
        it.second->RegisterNetlinkCallbacks(callbacks_);
        if (it.second->Start() != 0) {
            NETNATIVE_LOGE("Start netlink listener failed");
            return NetlinkResult::ERROR;
        }
    }
    return NetlinkResult::OK;
}

int32_t NetlinkManager::StopListener()
{
    for (auto &it : distributorMap_) {
        if (it.second == nullptr) {
            continue;
        }
#ifdef FEATURE_NET_FIREWALL_ENABLE
        if (it.first == NETLINK_NETFILTER) {
            auto paramIt = distributorParamList_.find(it.first);
            uint16_t groupId =
                (paramIt != distributorParamList_.end()) ? static_cast<uint16_t>(paramIt->second.groups) : 0;
            int32_t socketFd = it.second->GetSocketFd();
            if (socketFd >= 0 && !SendNflogUnbind(socketFd, groupId)) {
                NETNATIVE_LOGW("NFLOG unbinding failed before stopping the listener. group:%{public}u", groupId);
            }
        }
#endif
        if (it.second->Stop() != 0) {
            NETNATIVE_LOGE("Stop netlink listener failed");
            return NetlinkResult::ERROR;
        }
    }
    return NetlinkResult::OK;
}

int32_t NetlinkManager::RegisterNetlinkCallback(sptr<NetsysNative::INotifyCallback> callback)
{
    std::lock_guard<std::mutex> lock(linkCallbackMutex_);
    if (callback == nullptr) {
        NETNATIVE_LOGE("callback is nullptr");
        return NetlinkResult::ERR_NULL_PTR;
    }
    for (const auto &cb : *callbacks_) {
        if (cb->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            NETNATIVE_LOGI("callback is already registered");
            return NetlinkResult::OK;
        }
    }
    callbacks_->push_back(callback);
    NETNATIVE_LOGI("callback is registered successfully current size is %{public}zu", callbacks_->size());
    return NetlinkResult::OK;
}

int32_t NetlinkManager::UnregisterNetlinkCallback(sptr<NetsysNative::INotifyCallback> callback)
{
    std::lock_guard<std::mutex> lock(linkCallbackMutex_);
    if (callback == nullptr) {
        NETNATIVE_LOGE("callback is nullptr");
        return NetlinkResult::ERR_NULL_PTR;
    }
    for (auto it = callbacks_->begin(); it != callbacks_->end(); ++it) {
        if ((*it)->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            callbacks_->erase(it);
            NETNATIVE_LOGI("callback is unregistered successfully");
            return NetlinkResult::OK;
        }
    }
    NETNATIVE_LOGI("callback has not registered current callback number is %{public}zu", callbacks_->size());
    return NetlinkResult::ERR_INVALID_PARAM;
}
} // namespace nmd
} // namespace OHOS
