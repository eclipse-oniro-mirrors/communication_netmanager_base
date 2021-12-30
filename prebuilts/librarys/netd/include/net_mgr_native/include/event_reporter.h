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

#ifndef INCLUDE_EVENT_REPORTER_H__
#define INCLUDE_EVENT_REPORTER_H__
#include <string>

namespace OHOS {
namespace nmd {
typedef struct inetd_unsolicited_event_listener {
    void (*onInterfaceAddressUpdated)(const std::string &addr, const std::string &ifName, int flags, int scope);
    void (*onInterfaceAddressRemoved)(const std::string &addr, const std::string &ifName, int flags, int scope);
    void (*onInterfaceAdded)(const std::string &ifName);
    void (*onInterfaceRemoved)(const std::string &ifName);
    void (*onInterfaceChanged)(const std::string &ifName, bool up);
    void (*onInterfaceLinkStateChanged)(const std::string &ifName, bool up);
    void (*onRouteChanged)(
        bool updated, const std::string &route, const std::string &gateway, const std::string &ifName);
} inetd_unsolicited_event_listener;

class event_reporter {
public:
    event_reporter() = default;
    void registerEventListener(inetd_unsolicited_event_listener &listener);
    inetd_unsolicited_event_listener getListener()
    {
        return this->listener_;
    }
    ~event_reporter();

private:
    inetd_unsolicited_event_listener listener_;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_EVENT_REPORTER_H__