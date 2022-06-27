/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef _NETSYS_NETLINK_PROCESSOR_H
#define _NETSYS_NETLINK_PROCESSOR_H

#include "i_notify_callback.h"
#include "netlink_message_decoder.h"
#include "netlink_native_listener.h"

namespace OHOS {
namespace nmd {
class NetlinkProcessor : public NetlinkNativeListener {
public:
    NetlinkProcessor(std::shared_ptr<std::vector<sptr<NetsysNative::INotifyCallback>>> callback,
                     int32_t listenerSocket,
                     int32_t format);
    virtual ~NetlinkProcessor() = default;

    int32_t Start();
    int32_t Stop();

protected:
    virtual void OnEvent(std::shared_ptr<NetlinkMessageDecoder> message);

private:
    void OnStateChange(const std::shared_ptr<NetlinkMessageDecoder> message);

    void HandleAddressChange(const std::shared_ptr<NetlinkMessageDecoder> &message);
    void HandleRouteChange(const std::shared_ptr<NetlinkMessageDecoder> &message);
    void HandleRndssChange(const std::shared_ptr<NetlinkMessageDecoder> &message);
    void HandleSubSysNet(const std::shared_ptr<NetlinkMessageDecoder> &message);
    void HandleSubSysIdLetimer(const std::shared_ptr<NetlinkMessageDecoder> &message);
    void HandleSubSysQlog(const std::shared_ptr<NetlinkMessageDecoder> &message);
    void HandleSubSysStrict(const std::shared_ptr<NetlinkMessageDecoder> &message);

    void OnInterfaceAdd(const std::string &ifName);
    void OnInterfaceRemove(const std::string &ifName);
    void OnInterfaceChange(const std::string &ifName, bool isUp);
    void OnInterfaceLinkStateChange(const std::string &ifName, bool isUp);
    void OnQuotaLimitReache(const std::string &labelName, const std::string &ifName);
    void OnInterfaceClassActivityChange(int32_t label, bool isActive, int64_t timestamp, int32_t uid);
    void OnInterfaceAddressUpdate(const std::string &addr, const std::string &ifName, int32_t flags, int32_t scope);
    void OnInterfaceAddressRemove(const std::string &addr, const std::string &ifName, int32_t flags, int32_t scope);
    void OnInterfaceDnsServersUpdate(const std::string &ifName,
                                     int64_t lifetime,
                                     const std::vector<std::string> &servers);
    void OnRouteChange(bool updated, const std::string &route, const std::string &gateway, const std::string &ifName);
    void OnStrictCleartext(uid_t uid, const std::string &hex);

    std::shared_ptr<std::vector<sptr<NetsysNative::INotifyCallback>>> netlinkCallbacks_;
};
} // namespace nmd
} // namespace OHOS

#endif // _NETSYS_NETLINK_PROCESSOR_H
