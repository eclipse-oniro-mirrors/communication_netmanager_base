/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "i_notify_callback.h"
#include "netlink_native_listener.h"
#include "netlink_processor.h"

namespace OHOS {
namespace nmd {
class NetlinkManager {
public:
    static constexpr uint32_t NFLOG_QUOTA_GROUP = 1;
    static constexpr uint32_t NETFILTER_STRICT_GROUP = 2;
    static constexpr uint32_t NFLOG_WAKEUP_GROUP = 3;
    static constexpr uint32_t UEVENT_GROUP = 0xffffffff;

    static int32_t GetPid()
    {
        return pid_;
    }

    static void SetPid(int32_t pid)
    {
        pid_ = pid;
    }

    explicit NetlinkManager(int32_t pid);
    ~NetlinkManager();

    int32_t StartListener();
    int32_t StopListener();
    int32_t RegisterNetlinkCallback(sptr<NetsysNative::INotifyCallback> callback);
    int32_t UnRegisterNetlinkCallback(sptr<NetsysNative::INotifyCallback> callback);

private:
    static int32_t pid_;
    int32_t ueventSocket_ = -1;
    int32_t routeSocket_ = -1;
    int32_t quotaSocket_ = -1;
    int32_t strictSocket_ = -1;
    std::unique_ptr<NetlinkProcessor> ueventProc_ = nullptr;
    std::unique_ptr<NetlinkProcessor> routeProc_ = nullptr;
    std::unique_ptr<NetlinkProcessor> quotaProc_ = nullptr;
    std::unique_ptr<NetlinkProcessor> strictProc_ = nullptr;
    std::shared_ptr<std::vector<sptr<NetsysNative::INotifyCallback>>> netlinkCallbacks_;

    std::unique_ptr<NetlinkProcessor> SetSocket(int32_t &sock, int32_t netlinkType, uint32_t groups, int32_t format,
                                                bool configNflog);
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_NETLINK_MANAGER_H__
