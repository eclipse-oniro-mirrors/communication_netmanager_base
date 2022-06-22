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

#ifndef NET_CONN_NET_REQUEST_H
#define NET_CONN_NET_REQUEST_H

#include <string>
#include <vector>
#include "i_net_conn_callback.h"
#include "net_specifier.h"
#include "net_conn_async.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t DEFAULT_REQUEST_ID = 0;
class NetRequest : public virtual RefBase {
public:
    NetRequest(const sptr<NetSpecifier> &specifier,
        const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS, NetConnAsync& async);
    
    ~NetRequest();
    
    uint32_t GetId() const;

    void SetId(uint32_t reqId);

    void SetNetSupplierId(uint32_t supplierId);

    uint32_t GetNetSupplierId() const;

    sptr<NetSpecifier> GetNetSpecifier() const;

    sptr<INetConnCallback> GetNetConnCallback() const;

    void CallbackForNetAvailable(sptr<NetHandle> &netHandle);

    void CallbackForNetCapabilitiesChanged(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap);

    void CallbackForNetConnectionPropertiesChanged(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info);

    void CallbackForNetLost(sptr<NetHandle> &netHandle);

    void CallbackForNetUnavailable();

    void CallbackForNetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked);

private:
    void OnRequestTimeout();

private:
    uint32_t id_ {0};
    uint32_t supplierId_ {0};
    sptr<NetSpecifier> netSpecifier_;
    sptr<INetConnCallback> netConnCallback_;
    NetConnAsync& async_;
    std::shared_ptr<Scheduler::Task> timeTask_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_NET_REQUEST_H
