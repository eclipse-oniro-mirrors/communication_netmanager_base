/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "net_pac_manager.h"
#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <gtest/gtest.h>

constexpr int32_t PAC_QUERY_ITERATIONS = 100;
constexpr int32_t PAC_SCRIPT_RELOAD_ITERATIONS = 30;
constexpr int32_t PAC_SCRIPT_VARIANT_COUNT = 2;

using namespace OHOS::NetManagerStandard;
using namespace testing::ext;

HWTEST(NetPacManagerWifiTest, LoadDownloadedScriptAndResolve, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string pacScript = R"(
        function FindProxyForURL(url, host) {
            return "PROXY wifi.test:8080";
        }
    )";
    ASSERT_TRUE(manager->LoadPACScript(pacScript));

    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "PROXY wifi.test:8080");
}

HWTEST(NetPacManagerWifiTest, ReplaceDownloadedScriptChangesRules, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string script1 = R"(
        function FindProxyForURL(url, host) {
            return "PROXY old.test:80";
        }
    )";
    std::string script2 = R"(
        function FindProxyForURL(url, host) {
            return "DIRECT";
        }
    )";
    ASSERT_TRUE(manager->LoadPACScript(script1));
    ASSERT_TRUE(manager->LoadPACScript(script2));

    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "DIRECT");
}

HWTEST(NetPacManagerWifiTest, InvalidDownloadClearsOldRulesAndValidScriptRecovers, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string script1 = R"(
        function FindProxyForURL(url, host) {
            return "PROXY old.test:80";
        }
    )";
    std::string invalidScript = "function FindProxyForURL( {";
    std::string script2 = R"(
        function FindProxyForURL(url, host) {
            return "DIRECT";
        }
    )";
    ASSERT_TRUE(manager->LoadPACScript(script1));
    EXPECT_FALSE(manager->LoadPACScript(invalidScript));

    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_SCRIPT_DOWNLOAD_ERROR);
    EXPECT_TRUE(proxy.empty());
    ASSERT_TRUE(manager->LoadPACScript(script2));
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "DIRECT");
}

HWTEST(NetPacManagerWifiTest, ValidWifiScriptPreventsLazyReloadOfFailedGlobalUrl, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string pacScript = R"(
        function FindProxyForURL(url, host) {
            return "DIRECT";
        }
    )";
    EXPECT_FALSE(manager->InitPACScriptWithURL("invalid-scheme://global/proxy.pac"));
    ASSERT_TRUE(manager->LoadPACScript(pacScript));

    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "DIRECT");
}

HWTEST(NetPacManagerWifiTest, LoadedScriptWithoutFindFunctionReturnsFunctionError, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string pacScript = "var noProxyFunction = 1;";
    ASSERT_TRUE(manager->LoadPACScript(pacScript));

    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_SCRIPT_FUNCTION_ERROR);
    EXPECT_TRUE(proxy.empty());
}

HWTEST(NetPacManagerWifiTest, LoadedScriptRuntimeAndCallErrorsAreReported, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string script1 = "throw new Error('script failed');";
    std::string script2 = R"(
        function FindProxyForURL(url, host) {
            throw new Error("call failed");
        }
    )";
    ASSERT_TRUE(manager->LoadPACScript(script1));

    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_SCRIPT_RUN_ERROR);
    ASSERT_TRUE(manager->LoadPACScript(script2));
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_SCRIPT_CALL_ERROR);
    EXPECT_TRUE(proxy.empty());
}

HWTEST(NetPacManagerWifiTest, DownloadFailureDoesNotModifyLoadedEngine, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string pacScript = R"(
        function FindProxyForURL(url, host) {
            return "DIRECT";
        }
    )";
    ASSERT_TRUE(manager->LoadPACScript(pacScript));

    std::string downloaded;
    EXPECT_FALSE(NetPACManager::DownloadPACScript("", downloaded));
    EXPECT_TRUE(downloaded.empty());
    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://example.test", "example.test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "DIRECT");
}

HWTEST(NetPacManagerWifiTest, ConcurrentLoadAndResolveUseTheSameEngineLock, TestSize.Level1)
{
    const auto manager = std::make_shared<NetPACManager>();
    std::string script1 = R"(
        function FindProxyForURL(url, host) {
            return "PROXY first.test:80";
        }
    )";
    std::string script2 = R"(
        function FindProxyForURL(url, host) {
            return "PROXY second.test:80";
        }
    )";
    ASSERT_TRUE(manager->LoadPACScript(script1));

    std::promise<void> start;
    auto ready = start.get_future().share();
    std::atomic<bool> succeeded{true};
    std::thread reader([manager, ready, &succeeded]() {
        ready.wait();
        for (int i = 0; i < PAC_QUERY_ITERATIONS; ++i) {
            std::string proxy;
            if (manager->FindProxyForURL("http://example.test", "example.test", proxy) != PAC_OK ||
                (proxy != "PROXY first.test:80" && proxy != "PROXY second.test:80")) {
                succeeded = false;
            }
        }
    });
    start.set_value();
    for (int i = 0; i < PAC_SCRIPT_RELOAD_ITERATIONS; ++i) {
        if (!manager->LoadPACScript(i % PAC_SCRIPT_VARIANT_COUNT == 0 ? script2 : script1)) {
            succeeded = false;
        }
    }
    reader.join();
    EXPECT_TRUE(succeeded);
}
