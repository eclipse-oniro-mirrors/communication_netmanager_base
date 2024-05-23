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

HWTEST_F(DnsResolvListenTest, SetResolverConfigTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("SetResolverConfigTest001 enter");
    DnsParamCache dnsParCache;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
    servers.resize(MAX_SERVER_NUM + 1);
    uint16_t netId = 1;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    int32_t ret = dnsParCache.SetResolverConfig(netId, baseTimeoutMsec, retryCount, servers, domains);
    EXPECT_EQ(ret, -ENOENT);
}

HWTEST_F(DnsResolvListenTest, NetSysGetResolvConfTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("NetSysGetResolvConf001 enter");
    void *handle = dlopen(DNS_SO_PATH, RTLD_LAZY);
    if (handle == NULL) {
        NETNATIVE_LOGI("StartListenTest002 dlopen err %{public}s", dlerror());
        return;
    }
    GetConfig func = (GetConfig)dlsym(handle, "NetSysGetResolvConf");
    if (func == NULL) {
        NETNATIVE_LOGI("dlsym err %{public}s\n", dlerror());
        return;
    }

    ResolvConfig config = {0};
    int ret = func(0, &config);
    dlclose(handle);
    EXPECT_EQ(ret, -ENOENT);
}

HWTEST_F(DnsResolvListenTest, NetSysGetResolvConfTest002, TestSize.Level1)
{
    NETNATIVE_LOGI("StartListen enter");
    DnsResolvListen dnsResolvListen;
    dnsResolvListen.StartListen();

    NETNATIVE_LOGI("NetSysGetResolvConf002 enter");
    void *handle = dlopen(DNS_SO_PATH, RTLD_LAZY);
    if (handle == NULL) {
        NETNATIVE_LOGI("StartListenTest002 dlopen err %{public}s", dlerror());
        return;
    }
    GetConfig func = (GetConfig)dlsym(handle, "NetSysGetResolvConf");
    if (func == NULL) {
        NETNATIVE_LOGI("dlsym err %{public}s\n", dlerror());
        return;
    }

    ResolvConfig config = {0};
    int ret = func(3, &config);
    dlclose(handle);
    EXPECT_EQ(ret, -ENOENT);
}

HWTEST_F(DnsResolvListenTest, ProcSetCacheCommandTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("ProcSetCacheCommandTest001 enter");
    void *handle = dlopen(DNS_SO_PATH, RTLD_LAZY);
    if (handle == NULL) {
        NETNATIVE_LOGI("ProcSetCacheCommandTest001 dlopen err %{public}s", dlerror());
        return;
    }
    SetCache func = (SetCache)dlsym(handle, "NetSysSetResolvCache");
    if (func == NULL) {
        NETNATIVE_LOGI("ProcSetCacheCommandTest001 dlsym err %{public}s\n", dlerror());
        return;
    }
    std::string host = "www.1234.com";
    std::string serv;
    struct addrinfo *hint = nullptr;
    struct addrinfo *res = nullptr;
    struct ParamWrapper param = {const_cast<char *>(host.c_str()), const_cast<char *>(serv.c_str()),
                                 (struct addrinfo *)hint};
    int32_t ret = func(0, param, res);
    EXPECT_NE(ret, 0);
    dlclose(handle);
}

HWTEST_F(DnsResolvListenTest, ProcGetCacheCommandTest001, TestSize.Level1)
{
    NETNATIVE_LOGI("ProcGetCacheCommandTest001 enter");
    void *handle = dlopen(DNS_SO_PATH, RTLD_LAZY);
    if (handle == NULL) {
        NETNATIVE_LOGI("ProcGetCacheCommandTest001 dlopen err %{public}s", dlerror());
        return;
    }
    GetCache func = (GetCache)dlsym(handle, "NetSysGetResolvCache");
    if (func == NULL) {
        NETNATIVE_LOGI("ProcGetCacheCommandTest001 dlsym err %{public}s\n", dlerror());
        return;
    }
    std::string host = "www.1234.com";
    std::string serv;
    struct addrinfo *hint = nullptr;
    struct ParamWrapper param = {const_cast<char *>(host.c_str()), const_cast<char *>(serv.c_str()),
                                 (struct addrinfo *)hint};
    uint32_t num = 0;
    struct AddrInfo addr_info[MAX_RESULTS] = {{0}};
    int32_t ret = func(0, param, addr_info, &num);
    EXPECT_EQ(ret, NetManagerStandard::NETSYS_SUCCESS);
    dlclose(handle);
}


HWTEST_F(DnsResolvListenTest, ProcGetConfigCommand001, TestSize.Level1)
{
    instance_->ProcGetConfigCommand(CLIENT_SOCK_FD, static_cast<uint16_t>(NET_ID));
    ASSERT_EQ(instance_->serverSockFd_, -1);
}

HWTEST_F(DnsResolvListenTest, ProcGetKeyForCache001, TestSize.Level1)
{
    char name[MAX_HOST_NAME_LEN] = {0};
    auto ret = instance_->ProcGetKeyForCache(CLIENT_SOCK_FD, name);
    instance_->ProcSetCacheCommand(CLIENT_SOCK_FD, static_cast<uint16_t>(NET_ID));
    instance_->ProcGetCacheCommand(CLIENT_SOCK_FD, static_cast<uint16_t>(NET_ID));
    instance_->ProcCommand(CLIENT_SOCK_FD);
    EXPECT_EQ(ret, NetManagerStandard::NETMANAGER_ERROR);
}
} // namespace NetsysNative
} // namespace OHOS

