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

#ifndef NET_POLICY_CALLBACK_TEST_H
#define NET_POLICY_CALLBACK_TEST_H

#include <mutex>
#include <condition_variable>

#include "net_policy_callback_stub.h"
#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyCallbackTest : public NetPolicyCallbackStub {
public:
    NetPolicyCallbackTest();
    virtual ~NetPolicyCallbackTest() override;
    int32_t NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy) override;
    int32_t NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies) override;
    int32_t NetStrategySwitch(const std::string &simId, bool enable) override;
    int32_t NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow) override;
    void WaitFor(int32_t timeoutSecond);

    NetUidPolicy GetPolicy() const
    {
        return uidPolicy_;
    }
    uint32_t GetUid() const
    {
        return uid_;
    }
private:
    void NotifyAll();
    NetUidPolicy uidPolicy_ = NetUidPolicy::NET_POLICY_NONE;
    uint32_t uid_ = 0;
    bool isBackgroundPolicyAllow_ = true;
    std::mutex callbackMutex_;
    std::condition_variable cv_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CALLBACK_TEST_H
