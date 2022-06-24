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
    /**
     * Construct a new NetRequest
     *
     * @param specifier Used to satisfiy to net supplier
     * @param callback Net connection callback
     * @param timeoutMS NetUnavailable will be called if no supplier satisfied duraing timeoutMS milliseconds
     * @param async Async callback
     */
    NetRequest(const sptr<NetSpecifier> &specifier, const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS,
               NetConnAsync &async);

    /**
     * Destroy NetRequest
     *
     */
    ~NetRequest();

    /**
     * Get the request id
     *
     * @return uint32_t Request id
     */
    uint32_t GetId() const;

    /**
     * Set the request id
     *
     * @param reqId Request id to set
     */
    void SetId(uint32_t reqId);

    /**
     * Set the NetSupplier Id, means this net supplier is satisfied to the request, if supplierId is valid,
     *        timer to callback NetUnavailable will be canceled
     *
     * @param supplierId
     */
    void SetNetSupplierId(uint32_t supplierId);

    /**
     * Get the satisfied NetSupplier's id
     *
     * @return uint32_t
     */
    uint32_t GetNetSupplierId() const;

    /**
     * Get the NetSpecifier
     *
     * @return sptr<NetSpecifier>
     */
    sptr<NetSpecifier> GetNetSpecifier() const;

    /**
     * Get the NetConnCallback
     *
     * @return sptr<INetConnCallback>
     */
    sptr<INetConnCallback> GetNetConnCallback() const;

    /**
     * Callback NetAvailable
     *
     * @param netHandle Which network's available changed
     */
    void CallbackForNetAvailable(sptr<NetHandle> &netHandle);

    /**
     * Callback NetCapabilitiesChanged
     *
     * @param netHandle Which network's capabilities changed
     * @param netAllCap Network Capabilities
     */
    void CallbackForNetCapabilitiesChanged(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap);

    /**
     * Callback NetConnectionPropertiesChanged
     *
     * @param netHandle Which network's connection properties changed
     * @param info Network's connection properties
     */
    void CallbackForNetConnectionPropertiesChanged(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info);

    /**
     * Callback NetLost
     *
     * @param netHandle Which network was lost
     */
    void CallbackForNetLost(sptr<NetHandle> &netHandle);

    /**
     * Callback Unavailable
     *
     */
    void CallbackForNetUnavailable();

    /**
     * Callback NetBlockStatusChange
     *
     * @param netHandle  Which network's block status changed
     * @param blocked Network block status
     */
    void CallbackForNetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked);

private:
    void OnRequestTimeout();

private:
    uint32_t id_{0};
    uint32_t supplierId_{0};
    sptr<NetSpecifier> netSpecifier_;
    sptr<INetConnCallback> netConnCallback_;
    NetConnAsync &async_;
    std::shared_ptr<Scheduler::Task> timeTask_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_NET_REQUEST_H
