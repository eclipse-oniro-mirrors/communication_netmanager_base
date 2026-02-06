/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "dns_config_client.h"
#include "dns_param_cache.h"
#include "dns_resolv_listen.h"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace NetsysNative {
using namespace testing::ext;
using namespace OHOS::nmd;
constexpr const char *DNS_SO_PATH = "libnetsys_client.z.so";
static constexpr const int32_t CLIENT_SOCK_FD = 99999;
static constexpr const uint32_t NET_ID = 99999;
std::shared_ptr<DnsResolvListen> instance_ = nullptr;

typedef int32_t (*GetConfig)(uint16_t netId, struct ResolvConfig *config);
typedef int32_t (*GetConfigExt)(uint16_t netId, struct ResolvConfigExt *config);
typedef int32_t (*GetCache)(uint16_t netId, struct ParamWrapper param, struct AddrInfo addr_info[MAX_RESULTS],
                            uint32_t *num);

typedef int32_t (*SetCache)(uint16_t netId, struct ParamWrapper param, struct addrinfo *res);

class DnsResolvListenTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DnsResolvListenTest::SetUpTestCase() {}

void DnsResolvListenTest::TearDownTestCase() {}

void DnsResolvListenTest::SetUp()
{
    instance_ = std::make_shared<DnsResolvListen>();
}

void DnsResolvListenTest::TearDown() {}

HWTEST_F(DnsResolvListenTest, NetSysGetResolvConfTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NetSysGetResolvConf001 enter");
    void *handle = dlopen(DNS_SO_PATH, RTLD_LAZY);
    ASSERT_NE(handle, NULL);
    GetConfig func = (GetConfig)dlsym(handle, "NetSysGetResolvConf");
    ASSERT_NE(func, NULL);

    ResolvConfig config = {0};
    int ret = func(0, &config);
    dlclose(handle);
    EXPECT_GE(ret, -ENOENT);
}

HWTEST_F(DnsResolvListenTest, NetSysGetResolvConfExtTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NetSysGetResolvConfExtTest001 enter");
    void *handle = dlopen(DNS_SO_PATH, RTLD_LAZY);
    ASSERT_NE(handle, NULL);
    GetConfigExt func = (GetConfigExt)dlsym(handle, "NetSysGetResolvConf");
    EXPECT_NE(func, NULL);
    ResolvConfigExt config = {0};
    int ret = func(0, &config);
    dlclose(handle);
}
} // namespace NetsysNative
} // namespace OHOS
