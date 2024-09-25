/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <arpa/inet.h>

#define private public

#include "bpf_netfirewall.h"

using namespace testing::ext;
using namespace OHOS::NetManagerStandard;

class NetsysBpfNetFirewallTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetsysBpfNetFirewallTest::SetUpTestCase() {}

void NetsysBpfNetFirewallTest::TearDownTestCase() {}

void NetsysBpfNetFirewallTest::SetUp() {}

void NetsysBpfNetFirewallTest::TearDown() {}

HWTEST_F(NetsysBpfNetFirewallTest, AddDomainCache001, TestSize.Level0)
{
    std::shared_ptr<NetsysBpfNetFirewall> bpfNet = NetsysBpfNetFirewall::GetInstance();
    NetAddrInfo netInfo;
    netInfo.aiFamily = AF_INET;
    size_t size = bpfNet->domainCache_.size();
    inet_pton(AF_INET, "192.168.8.116", &netInfo.aiAddr.sin);
    bpfNet->AddDomainCache(netInfo);
    EXPECT_EQ(bpfNet->domainCache_.size(), size + 1);
    size++;
    netInfo.aiFamily = AF_INET6;
    inet_pton(AF_INET6, "fe80::6bec:e9b9:a1df:f69d", &netInfo.aiAddr.sin6);
    bpfNet->AddDomainCache(netInfo);
    EXPECT_EQ(bpfNet->domainCache_.size(), size + 1);
}

HWTEST_F(NetsysBpfNetFirewallTest, ClearDomainCache001, TestSize.Level0)
{
    NetsysBpfNetFirewall::GetInstance()->ClearDomainCache();
    EXPECT_EQ(NetsysBpfNetFirewall::GetInstance()->domainCache_.size(), 0);
}