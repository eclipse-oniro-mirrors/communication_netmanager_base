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

#include "net_conn_callback_test.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnCallbackTest::NetConnCallbackTest() {}

NetConnCallbackTest::~NetConnCallbackTest() {}

void NetConnCallbackTest::NotifyAll()
{
    std::unique_lock<std::mutex> callbackLock(callbackMutex_);
    cv_.notify_all();
}

void NetConnCallbackTest::WaitFor(int timeoutSecond)
{
    std::unique_lock<std::mutex> callbackLock(callbackMutex_);
    cv_.wait_for(callbackLock, std::chrono::seconds(timeoutSecond));
}

int32_t NetConnCallbackTest::NetAvailable(sptr<NetHandle> &netHandle)
{
    if (netHandle != nullptr) {
        return 0;
    }

    NETMGR_LOG_D("NetAvailable: netId = %{public}d", netHandle->GetNetId());
    NotifyAll();
    return 0;
}

int32_t NetConnCallbackTest::NetCapabilitiesChange(
    sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
{
    if (netHandle == nullptr || netAllCap == nullptr) {
        return 0;
    }

    NETMGR_LOG_D("NetCapabilitiesChange: netId = [%{public}d]", netHandle->GetNetId());
    NETMGR_LOG_D("[%{public}s]", netAllCap->ToString("|").c_str());
    NotifyAll();
    return 0;
}

int32_t NetConnCallbackTest::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetLinkInfo> &info)
{
    if (netHandle == nullptr || info == nullptr) {
        return 0;
    }
    NETMGR_LOG_D("NetConnectionPropertiesChange: netId = %{public}d info = %{public}s", netHandle->GetNetId(),
        info->ToString(" ").c_str());
    NotifyAll();
    return 0;
}

int32_t NetConnCallbackTest::NetLost(sptr<NetHandle> &netHandle)
{
    if (netHandle == nullptr) {
        return 0;
    }
    NETMGR_LOG_D("NetLost: netId = %{public}d", netHandle->GetNetId());
    NotifyAll();
    return 0;
}

int32_t NetConnCallbackTest::NetUnavailable()
{
    NETMGR_LOG_D("NetUnavailable");
    NotifyAll();
    return 0;
}

int32_t NetConnCallbackTest::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool isBlocked)
{
    NETMGR_LOG_D("NetConnCallbackTest::NetLost: netId = %{public}d bolcked = %{public}s",
        netHandle->GetNetId(), isBlocked ? "true" : "false");
    NotifyAll();
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
