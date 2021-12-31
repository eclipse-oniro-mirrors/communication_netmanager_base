/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_NETLINK_HANDLER_H__
#define INCLUDE_NETLINK_HANDLER_H__
#include <vector>
#include "event_reporter.h"
#include "netlink_listener.h"
#include "netlink_event.h"

namespace OHOS {
namespace nmd {
class netlink_handler : public netlink_listener {
public:
    void onEvent(std::shared_ptr<netlink_event> evt);

    void notifyInterfaceAdded(const std::string &ifName);
    void notifyInterfaceRemoved(const std::string &ifName);
    void notifyInterfaceChanged(const std::string &ifName, bool isUp);
    void notifyInterfaceLinkChanged(const std::string &ifName, bool isUp);
    void notifyAddressUpdated(const std::string &addr, const std::string &ifName, int flags, int scope);
    void notifyAddressRemoved(const std::string &addr, const std::string &ifName, int flags, int scope);
    void notifyRouteChange(
        bool updated, const std::string &route, const std::string &gateway, const std::string &ifName);

    int start();
    void stop();

    int getSock()
    {
        return this->socketFd_;
    }

    void setEventListener(const std::shared_ptr<event_reporter> &reporter)
    {
        this->reporter_ = reporter;
    }

    netlink_handler(int protocol, int pid);
    ~netlink_handler();

private:
    std::shared_ptr<event_reporter> reporter_;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_HANDLER_H__