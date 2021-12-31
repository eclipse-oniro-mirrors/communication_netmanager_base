/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "net_activate.h"

#include <atomic>

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<uint32_t> g_nextRequestId = 0;

NetActivate::NetActivate(const sptr<NetSpecifier> &specifier, const sptr<INetConnCallback> &callback)
    : netSpecifier_(specifier), netConnCallback_(callback)
{
    requestId_ = g_nextRequestId++;
}

bool NetActivate::MatchRequestAndNetwork(sptr<NetSupplier> supplier)
{
    NETMGR_LOG_E("MatchRequestAndNetwork Enter");
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is null");
        return false;
    }
    if (!CompareByNetworkIdent(supplier->GetNetSupplierIdent())) {
        NETMGR_LOG_D("supplier ident is not satisfy");
        return false;
    }
    if (!CompareByNetworkCapabilities(supplier->GetNetCapabilities())) {
        NETMGR_LOG_D("supplier capa is not satisfy");
        return false;
    }
    if (!CompareByNetworkNetType(static_cast<uint32_t>(supplier->GetNetSupplierType()))) {
        NETMGR_LOG_D("supplier net type not satisfy");
        return false;
    }
    return true;
}

bool NetActivate::CompareByNetworkIdent(const std::string &ident)
{
    if (ident.empty()) {
        return true;
    }
    if (ident == netSpecifier_->ident_) {
        return true;
    }
    return false;
}

bool NetActivate::CompareByNetworkCapabilities(const uint64_t &netCapabilities)
{
    if ((netCapabilities & netSpecifier_->netCapabilities_) != netSpecifier_->netCapabilities_) {
        return false;
    }
    return true;
}

bool NetActivate::CompareByNetworkNetType(uint32_t netType)
{
    if ((netSpecifier_->netType_ == NET_TYPE_UNKNOWN) || (netSpecifier_->netType_ == NET_TYPE_MAX)) {
        return true;
    }
    if (netType == netSpecifier_->netType_) {
        return true;
    }
    return false;
}

sptr<NetSpecifier>& NetActivate::GetNetSpecifier()
{
    return netSpecifier_;
}

uint32_t NetActivate::GetRequestId() const
{
    return requestId_;
}

sptr<NetSupplier> NetActivate::GetServiceSupply() const
{
    return netServiceSupplied_;
}

void NetActivate::SetServiceSupply(sptr<NetSupplier> netServiceSupplied)
{
    netServiceSupplied_ = netServiceSupplied;
}

sptr<INetConnCallback>& NetActivate::GetNetCallback()
{
    return netConnCallback_;
}
} // namespace NetManagerStandard
} // namespace OHOS