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

#ifndef WRAPPER_DISTRIBUTOR_H
#define WRAPPER_DISTRIBUTOR_H

#include "data_receiver.h"
#include "netsys_event_message.h"
#include "i_notify_callback.h"

namespace OHOS {
namespace nmd {
class WrapperDistributor {
public:
    WrapperDistributor(int32_t socket, const int32_t format);
    ~WrapperDistributor() = default;

    int32_t Start();
    int32_t Stop();
    int32_t
        RegisterNetlinkCallbacks(std::shared_ptr<std::vector<sptr<NetsysNative::INotifyCallback>>> netlinkCallbacks);

private:
    void HandleDecodeSuccess(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleStateChanged(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleAddressChange(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleRouteChange(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleRndssChange(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleSubSysNet(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleSubSysQlog(const std::shared_ptr<NetsysEventMessage> &message);
    void HandleSubSysStrict(const std::shared_ptr<NetsysEventMessage> &message);
    void NotifyInterfaceAdd(const std::string &ifName);
    void NotifyInterfaceRemove(const std::string &ifName);
    void NotifyInterfaceChange(const std::string &ifName, bool isUp);
    void NotifyInterfaceLinkStateChange(const std::string &ifName, bool isUp);
    void NotifyQuotaLimitReache(const std::string &labelName, const std::string &ifName);
    void NotifyInterfaceClassActivityChange(int32_t label, bool isActive, int64_t timestamp, int32_t uid);
    void NotifyInterfaceAddressUpdate(const std::string &addr, const std::string &ifName, int32_t flags, int32_t scope);
    void NotifyInterfaceAddressRemove(const std::string &addr, const std::string &ifName, int32_t flags, int32_t scope);
    void NotifyInterfaceDnsServersUpdate(const std::string &ifName, int64_t lifetime,
                                         const std::vector<std::string> &servers);
    void NotifyRouteChange(bool updated, const std::string &route, const std::string &gateway,
                           const std::string &ifName);
    void NotifyStrictCleartext(const std::string &uid, const std::string &hex);

    std::unique_ptr<DataReceiver> receiver_;
    std::shared_ptr<std::vector<sptr<NetsysNative::INotifyCallback>>> netlinkCallbacks_;
};
} // namespace nmd
} // namespace OHOS

#endif // WRAPPER_DISTRIBUTOR_H
