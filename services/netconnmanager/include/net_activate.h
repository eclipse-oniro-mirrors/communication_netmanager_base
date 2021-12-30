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

#ifndef NET_ACTIVATE_H
#define NET_ACTIVATE_H

#include <string>
#include <vector>
#include "i_net_conn_callback.h"
#include "net_specifier.h"
#include "net_supplier.h"

class NetSupplier;

namespace OHOS {
namespace NetManagerStandard {
class NetActivate : public virtual RefBase {
public:
    NetActivate(const sptr<NetSpecifier> &specifier, const sptr<INetConnCallback> &callback);
    ~NetActivate() = default;
    bool MatchRequestAndNetwork(sptr<NetSupplier> supplier);
    uint32_t GetRequestId() const;
    sptr<NetSupplier> GetServiceSupply() const;
    void SetServiceSupply(sptr<NetSupplier> netServiceSupplied);
    sptr<INetConnCallback>& GetNetCallback();
    sptr<NetSpecifier>& GetNetSpecifier();

private:
    bool CompareByNetworkIdent(const std::string &ident);
    bool CompareByNetworkCapabilities(const uint64_t &netCapabilities);
    bool CompareByNetworkNetType(uint32_t netType);

private:
    uint32_t requestId_ = 1;
    sptr<NetSpecifier> netSpecifier_ = nullptr;
    sptr<INetConnCallback> netConnCallback_ = nullptr;
    sptr<NetSupplier> netServiceSupplied_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_ACTIVATE_H