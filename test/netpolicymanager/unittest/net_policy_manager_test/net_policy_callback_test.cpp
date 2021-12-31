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

#include "net_policy_callback_test.h"
#include "net_policy_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyCallbackTest::NetPolicyCallbackTest() {}

NetPolicyCallbackTest::~NetPolicyCallbackTest() {}

void NetPolicyCallbackTest::NotifyAll()
{
    std::unique_lock<std::mutex> callbackLock(callbackMutex_);
    cv_.notify_all();
}

void NetPolicyCallbackTest::WaitFor(int32_t timeoutSecond)
{
    std::unique_lock<std::mutex> callbackLock(callbackMutex_);
    cv_.wait_for(callbackLock, std::chrono::seconds(timeoutSecond));
}

int32_t NetPolicyCallbackTest::NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOG_D("unittest NetUidPolicyChanged, uid:[%{public}d], policy:[%{public}d]",
        uid, static_cast<uint32_t>(policy));

    uid_ = uid;
    uidPolicy_ = policy;
    NotifyAll();

    return 0;
}

int32_t NetPolicyCallbackTest::NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicys)
{
    NETMGR_LOG_D("unittest NetCellularPolicyChanged, cellularPolicys.size:[%{public}d]",
        cellularPolicys.size());

    return 0;
}

int32_t NetPolicyCallbackTest::NetStrategySwitch(const std::string &subscriberId, bool enable)
{
    NETMGR_LOG_D("unittest NetStrategySwitch, subscriberId:[%{public}s], enable:[%{public}d]",
        subscriberId.c_str(), static_cast<uint32_t>(enable));

    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
