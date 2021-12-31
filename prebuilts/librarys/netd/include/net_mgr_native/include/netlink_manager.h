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

#ifndef INCLUDE_NETLINK_MANAGER_H__
#define INCLUDE_NETLINK_MANAGER_H__

#include <memory>
#include "event_reporter.h"
#include "netlink_handler.h"
namespace OHOS {
namespace nmd {
namespace listeners {
void defaultOnInterfaceAddressUpdated(const std::string &, const std::string &, int, int);
void defaultOnInterfaceAddressRemoved(const std::string &, const std::string &, int, int);
void defaultOnInterfaceAdded(const std::string &);
void defaultOnInterfaceRemoved(const std::string &);
void defaultOnInterfaceChanged(const std::string &, bool);
void defaultOnInterfaceLinkStateChanged(const std::string &, bool);
void defaultOnRouteChanged(bool, const std::string &, const std::string &, const std::string &);
} // namespace listeners
class netlink_manager {
public:
    void start();
    void stop();

    int getRouteSock()
    {
        return this->routeHandler_->getSock();
    }
    std::shared_ptr<netlink_handler> getRouteHandler()
    {
        return this->routeHandler_;
    }

    static int getPid()
    {
        return pid_;
    }
    static void setPid(int pid)
    {
        pid_ = pid;
    }
    static std::shared_ptr<event_reporter> getReporter()
    {
        return reporter_;
    }

    explicit netlink_manager(int pid);
    ~netlink_manager();

private:
    static int pid_;
    static std::shared_ptr<event_reporter> reporter_;

    std::shared_ptr<netlink_handler> routeHandler_;
    void startRouteHandler();
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_MANAGER_H__