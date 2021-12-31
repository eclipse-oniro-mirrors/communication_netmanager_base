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

#ifndef NET_CONN_CALLBACK_TEST_H
#define NET_CONN_CALLBACK_TEST_H

#include <condition_variable>
#include <mutex>

#include "net_conn_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnCallbackTest : public NetConnCallbackStub {
public:
    NetConnCallbackTest();
    virtual ~NetConnCallbackTest() override;
    int32_t NetConnStateChanged(const sptr<NetConnCallbackInfo> &info) override;
    int32_t NetAvailable(int32_t netId) override;
    int32_t NetCapabilitiesChange(int32_t netId, const uint64_t &netCap) override;
    int32_t NetConnectionPropertiesChange(int32_t netId, const sptr<NetLinkInfo> &info) override;
    int32_t NetLost(int32_t netId) override;
    void WaitFor(int timeoutSecond);

    int GetNetState() const
    {
        return netState_;
    }

private:
    void NotifyAll();
    int32_t netState_ = 0;
    std::mutex callbackMutex_;
    std::condition_variable cv_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_CALLBACK_TEST_H
