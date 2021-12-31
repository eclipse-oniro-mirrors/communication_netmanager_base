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

#ifndef INCLUDE_NETLINK_EVENT_H__
#define INCLUDE_NETLINK_EVENT_H__

#include <map>
#include <stddef.h>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
namespace OHOS {
namespace nmd {
enum class Action {
    Unknown = 0,
    Add,
    Remove,
    Change,
    LinkUp,
    LinkDown,
    AddressUpdated,
    AddressRemoved,
    RouteUpdated,
    RouteRemoved,
    NewRule,
    DelRule,
};
class netlink_event {
public:
    netlink_event() = default;
    bool parseInterfaceInfoInfoMessage(struct nlmsghdr *hdr);
    bool parseInterafaceAddressMessage(struct nlmsghdr *hdr);
    bool parseRouteMessage(struct nlmsghdr *hdr);
    bool parseRuleMessage(struct nlmsghdr *hdr);
    bool parseNetLinkMessage(char *buffer, ssize_t size);

    void setAction(Action action)
    {
        this->action_ = action;
    }
    Action getAction()
    {
        return this->action_;
    }

    void addParam(std::string key, std::string value)
    {
        this->params_.insert(std::pair<std::string, std::string>(key, value));
    }
    const char *findParam(const char *key);
    const char *rtMessageName(int type);

    ~netlink_event();

private:
    Action action_;
    std::map<std::string, std::string> params_;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_EVENT_H__