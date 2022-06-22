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
#include "net_request.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t MIN_REQUEST_ID = DEFAULT_REQUEST_ID + 1;
constexpr uint32_t MAX_REQUEST_ID = 0x7FFFFFFF;
static std::atomic<uint32_t> g_nextRequestId = DEFAULT_REQUEST_ID;
using TimeOutCallback = std::function<void()>;
NetRequest::NetRequest(const sptr<NetSpecifier> &specifier,
    const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS, NetConnAsync& async)
    :id_(g_nextRequestId++), netSpecifier_(specifier), netConnCallback_(callback), async_(async)
{
    if (id_ > MAX_REQUEST_ID) {
        id_ = MIN_REQUEST_ID;
    }

    if (timeoutMS > 0) {
        timeTask_ = async_.GetScheduler().DelayPost(
            std::bind(&NetRequest::OnRequestTimeout, this), timeoutMS);
    }
}

NetRequest::~NetRequest()
{
    if (timeTask_) {
        timeTask_->Cancel();
    }
}

uint32_t NetRequest::GetId() const
{
    return id_;
}

void NetRequest::SetId(uint32_t reqId)
{
    id_ = reqId;
}

void NetRequest::SetNetSupplierId(uint32_t supplierId)
{
    if (supplierId) {
        if (timeTask_) {
            timeTask_->Cancel();
            timeTask_ = nullptr;
        }
    }
    supplierId_ = supplierId;
}

uint32_t NetRequest::GetNetSupplierId() const
{
    return supplierId_;
}

sptr<NetSpecifier> NetRequest::GetNetSpecifier() const
{
    return netSpecifier_;
}

sptr<INetConnCallback> NetRequest::GetNetConnCallback() const
{
    return netConnCallback_;
}

void NetRequest::CallbackForNetAvailable(sptr<NetHandle> &netHandle)
{
    if (netConnCallback_) {
        netConnCallback_->NetAvailable(netHandle);
    }
}

void NetRequest::CallbackForNetUnavailable()
{
    if (netConnCallback_) {
        netConnCallback_->NetUnavailable();
    }
}

void NetRequest::CallbackForNetCapabilitiesChanged(
    sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCaps)
{
    if (netConnCallback_) {
        netConnCallback_->NetCapabilitiesChange(netHandle, netAllCaps);
    }
}

void NetRequest::CallbackForNetConnectionPropertiesChanged(
    sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info)
{
    if (netConnCallback_) {
        netConnCallback_->NetConnectionPropertiesChange(netHandle, info);
    }
}

void NetRequest::CallbackForNetLost(sptr<NetHandle> &netHandle)
{
    if (netConnCallback_) {
        netConnCallback_->NetLost(netHandle);
    }
}

void NetRequest::CallbackForNetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    if (netConnCallback_) {
        netConnCallback_->NetBlockStatusChange(netHandle, blocked);
    }
}

void NetRequest::OnRequestTimeout()
{
    if (netConnCallback_) {
        NETMGR_LOG_I("NetRequest[%{public}d] request timeout", id_);
        netConnCallback_->NetUnavailable();
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
