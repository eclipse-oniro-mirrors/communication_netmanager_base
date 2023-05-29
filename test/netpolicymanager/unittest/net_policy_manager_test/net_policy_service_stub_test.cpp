/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <vector>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "net_policy_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
#define DTEST_LOG std::cout << __func__ << ":" << __LINE__ << ":"
class MockNetPolicyServiceStubTest : public NetPolicyServiceStub {
public:
    MockNetPolicyServiceStubTest() = default;
    ~MockNetPolicyServiceStubTest() = default;
    int32_t SetPolicyByUid(uint32_t uid, uint32_t policy) override
    {
        return 0;
    }

    int32_t GetPolicyByUid(uint32_t uid, uint32_t &policy) override
    {
        return 0;
    }

    int32_t GetUidsByPolicy(uint32_t policy, std::vector<uint32_t> &uids) override
    {
        return 0;
    }

    int32_t IsUidNetAllowed(uint32_t uid, bool metered, bool &isAllowed) override
    {
        return 0;
    }

    int32_t IsUidNetAllowed(uint32_t uid, const std::string &ifaceName, bool &isAllowed) override
    {
        return 0;
    }

    int32_t RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) override
    {
        return 0;
    }

    int32_t UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback) override
    {
        return 0;
    }

    int32_t SetNetQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies) override
    {
        return 0;
    }

    int32_t GetNetQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies) override
    {
        return 0;
    }

    int32_t UpdateRemindPolicy(int32_t netType, const std::string &iccid, uint32_t remindType) override
    {
        return 0;
    }

    int32_t SetDeviceIdleAllowedList(const std::vector<uint32_t> &uids, bool isAllowed) override
    {
        return 0;
    }

    int32_t GetDeviceIdleAllowedList(std::vector<uint32_t> &uids) override
    {
        return 0;
    }

    int32_t SetDeviceIdlePolicy(bool enable) override
    {
        return 0;
    }

    int32_t ResetPolicies(const std::string &iccid) override
    {
        return 0;
    }

    int32_t SetBackgroundPolicy(bool isAllowed) override
    {
        return 0;
    }

    int32_t GetBackgroundPolicy(bool &backgroundPolicy) override
    {
        return 0;
    }

    int32_t GetBackgroundPolicyByUid(uint32_t uid, uint32_t &backgroundPolicyOfUid) override
    {
        return 0;
    }

    int32_t GetPowerSaveAllowedList(std::vector<uint32_t> &uids) override
    {
        return 0;
    }

    int32_t SetPowerSaveAllowedList(const std::vector<uint32_t> &uids, bool isAllowed) override
    {
        return 0;
    }

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        bool byPassPolicyPermission = false;
        if (!data.ReadBool(byPassPolicyPermission)) {
            return NETMANAGER_ERR_READ_DATA_FAIL;
        }

        if (!byPassPolicyPermission) {
            return NetPolicyServiceStub::OnRemoteRequest(code, data, reply, option);
        }

        auto itFunc = memberFuncMap_.find(code);
        int32_t result = NETMANAGER_SUCCESS;
        if (itFunc != memberFuncMap_.end()) {
            auto requestFunc = itFunc->second;
            if (requestFunc != nullptr) {
                handler_->PostSyncTask(
                    [this, &data, &reply, &requestFunc, &result]() { result = (this->*requestFunc)(data, reply); },
                    AppExecFwk::EventQueue::Priority::HIGH);
                return result;
            }
        }

        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
};

} // namespace

using namespace testing::ext;
class NetPolicyServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline sptr<NetPolicyServiceStub> instance_ = new (std::nothrow) MockNetPolicyServiceStubTest();
};

void NetPolicyServiceStubTest::SetUpTestCase() {}

void NetPolicyServiceStubTest::TearDownTestCase() { instance_ = nullptr; }

void NetPolicyServiceStubTest::SetUp() {}

void NetPolicyServiceStubTest::TearDown() {}

HWTEST_F(NetPolicyServiceStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteBool(false);
    int32_t ret = instance_->OnRemoteRequest(INetPolicyService::CMD_NPS_END, data, reply, option);
    EXPECT_EQ(ret, NETMANAGER_ERR_DESCRIPTOR_MISMATCH);
}

} // namespace NetManagerStandard
} // namespace OHOS