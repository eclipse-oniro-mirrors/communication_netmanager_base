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

#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <poll.h>
#include <set>
#include <shared_mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "net_conn_service.h"
#include "net_manager_constants.h"
#include "net_pac_proxy_controller.h"
#include "net_supplier.h"
#include "network.h"

namespace OHOS {
namespace NetManagerStandard {
#ifdef NETMANAGER_ENABLE_PAC_PROXY
namespace {
using namespace testing::ext;
using Source = NetPacProxyController::PacSource;
using Decision = NetPacProxyController::PacDecision;
constexpr int32_t WIFI_NET_ID = 1001;
constexpr int32_t OTHER_NET_ID = 1002;
constexpr uint16_t MANUAL_PROXY_PORT = 8080;
constexpr int32_t LOCAL_PROXY_TEST_PORT = 12345;
constexpr int HTTP_OK = 200;
constexpr int HTTP_NOT_FOUND = 404;
constexpr int HTTP_UNAVAILABLE = 503;
constexpr int LISTEN_BACKLOG = 8;
constexpr int POLL_TIMEOUT_MS = 100;
constexpr int CLIENT_READ_TIMEOUT_SECONDS = 1;
constexpr size_t REQUEST_BUFFER_SIZE = 2048;
constexpr int EXPECTED_DOWNLOADS_AFTER_RELOAD = 2;
constexpr const char *WIFI_SCRIPT =
    "function FindProxyForURL(url, host) { return 'PROXY wifi.test:8080'; }";

HttpProxy PacProxy(const std::string &url)
{
    HttpProxy proxy;
    proxy.SetPacUrl(url);
    return proxy;
}

HttpProxy ManualProxy(const std::string &host = "manual.test")
{
    return HttpProxy(host, MANUAL_PROXY_PORT, {});
}

class PacHttpServer {
public:
    explicit PacHttpServer(std::string body = WIFI_SCRIPT, int status = HTTP_OK,
        std::function<void()> beforeReply = {})
        : body_(std::move(body)), status_(status), beforeReply_(std::move(beforeReply))
    {
    }

    ~PacHttpServer()
    {
        running_ = false;
        if (fd_ >= 0) {
            shutdown(fd_, SHUT_RDWR);
        }
        if (worker_.joinable()) {
            worker_.join();
        }
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    bool Start()
    {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0) {
            return false;
        }
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(fd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0 ||
            listen(fd_, LISTEN_BACKLOG) != 0) {
            return false;
        }
        socklen_t length = sizeof(address);
        if (getsockname(fd_, reinterpret_cast<sockaddr *>(&address), &length) != 0) {
            return false;
        }
        port_ = ntohs(address.sin_port);
        running_ = true;
        worker_ = std::thread([this]() { Serve(); });
        return true;
    }

    std::string Url(const std::string &path = "/wifi.pac") const
    {
        return "http://127.0.0.1:" + std::to_string(port_) + path;
    }

    int Requests() const
    {
        return requests_.load();
    }

private:
    void SendResponse(int client, const std::string &response) const
    {
        size_t sent = 0;
        while (sent < response.size()) {
            ssize_t count = send(client, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
            if (count <= 0) {
                return;
            }
            sent += static_cast<size_t>(count);
        }
    }

    void HandleClient(int client)
    {
        timeval timeout{CLIENT_READ_TIMEOUT_SECONDS, 0};
        if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
            return;
        }
        char buffer[REQUEST_BUFFER_SIZE];
        if (recv(client, buffer, sizeof(buffer), 0) <= 0) {
            return;
        }
        ++requests_;
        if (beforeReply_) {
            beforeReply_();
        }
        std::string response = "HTTP/1.1 " + std::to_string(status_) + " Test\r\nContent-Length: " +
            std::to_string(body_.size()) + "\r\nConnection: close\r\n\r\n" + body_;
        SendResponse(client, response);
    }

    void Serve()
    {
        while (running_) {
            pollfd event{fd_, POLLIN, 0};
            if (poll(&event, 1, POLL_TIMEOUT_MS) <= 0) {
                continue;
            }
            int client = accept(fd_, nullptr, nullptr);
            if (client < 0) {
                continue;
            }
            HandleClient(client);
            close(client);
        }
    }

    int fd_ = -1;
    uint16_t port_ = 0;
    std::string body_;
    int status_;
    std::function<void()> beforeReply_;
    std::atomic<bool> running_{false};
    std::atomic<int> requests_{0};
    std::thread worker_;
};

}

class NetPacProxyControllerTest : public testing::Test {
public:
    void SetUp() override
    {
        service_ = std::make_shared<NetConnService>();
        ASSERT_EQ(service_->GetActiveUserId(userId_), NETMANAGER_SUCCESS);
        SetGlobalProxy(HttpProxy());
        service_->netPACManager_ = std::make_shared<NetPACManager>();
    }

    void TearDown() override
    {
        WaitQueue();
        service_->pacController_.StopServer();
        service_->StopPacLocalProxyServer();
        service_.reset();
    }

protected:
    void SetGlobalProxy(HttpProxy proxy)
    {
        proxy.SetUserId(userId_);
        service_->globalHttpProxyCache_.EnsureInsert(userId_, proxy);
    }

    sptr<NetSupplier> SetNetwork(const HttpProxy &proxy, NetBearType bearer = BEARER_WIFI,
        int32_t netId = WIFI_NET_ID, bool connected = true)
    {
        auto supplier = sptr<NetSupplier>::MakeSptr(bearer, "wifiPacUnitTest",
            std::set<NetCap>{NET_CAPABILITY_INTERNET});
        auto network = std::make_shared<Network>(netId, supplier->GetSupplierId(), bearer, nullptr);
        network->netLinkInfo_.httpProxy_ = proxy;
        network->state_ = connected ? NET_CONN_STATE_CONNECTED : NET_CONN_STATE_DISCONNECTED;
        supplier->SetNetwork(network);
        {
            std::unique_lock<ffrt::shared_mutex> lock(service_->defaultNetSupplierMutex_);
            service_->defaultNetSupplier_ = supplier;
        }
        service_->netSuppliers_[supplier->GetSupplierId()] = supplier;
        return supplier;
    }

    void WaitQueue()
    {
        auto queue = service_->pacController_.queue_;
        if (queue != nullptr) {
            auto barrier = queue->submit_h([]() {});
            queue->wait(barrier);
        }
    }

    HttpProxy Effective(int32_t netId = INVALID_NET_ID)
    {
        HttpProxy proxy;
        EXPECT_EQ(service_->GetDefaultHttpProxy(netId, proxy), NETMANAGER_SUCCESS);
        return proxy;
    }

    std::shared_ptr<NetConnService> service_;
    int32_t userId_ = -1;
};

HWTEST_F(NetPacProxyControllerTest, DecisionEqualityIncludesSourceNetworkAndUrl, TestSize.Level1)
{
    Decision target{Source::WIFI, WIFI_NET_ID, "http://test/wifi.pac"};
    EXPECT_TRUE(target == target);
    EXPECT_FALSE(target == (Decision{Source::GLOBAL, WIFI_NET_ID, target.url}));
    EXPECT_FALSE(target == (Decision{Source::WIFI, OTHER_NET_ID, target.url}));
    EXPECT_FALSE(target == (Decision{Source::WIFI, WIFI_NET_ID, "http://test/other.pac"}));
}

HWTEST_F(NetPacProxyControllerTest, DecideRequiresConnectedDefaultWifiWithPac, TestSize.Level1)
{
    auto &controller = service_->pacController_;
    EXPECT_EQ(controller.Decide().source, Source::NONE);
    SetNetwork(PacProxy("http://test/wifi.pac"), BEARER_CELLULAR);
    EXPECT_EQ(controller.Decide().source, Source::NONE);
    auto supplier = SetNetwork(PacProxy("http://test/wifi.pac"), BEARER_WIFI, WIFI_NET_ID, false);
    EXPECT_EQ(controller.Decide().source, Source::NONE);
    supplier->SetNetwork(nullptr);
    EXPECT_EQ(controller.Decide().source, Source::NONE);
    SetNetwork(HttpProxy());
    EXPECT_EQ(controller.Decide().source, Source::NONE);
    SetNetwork(ManualProxy());
    EXPECT_EQ(controller.Decide().source, Source::NONE);
    SetNetwork(PacProxy("http://test/wifi.pac"));
    EXPECT_EQ(controller.Decide().source, Source::WIFI);
    EXPECT_EQ(controller.Decide().netId, WIFI_NET_ID);
    EXPECT_EQ(controller.Decide().url, "http://test/wifi.pac");
}

HWTEST_F(NetPacProxyControllerTest, ManualGlobalProxyHasPriority, TestSize.Level1)
{
    SetNetwork(PacProxy("http://test/wifi.pac"));
    SetGlobalProxy(ManualProxy());
    service_->pacController_.ApplyWifiPac();
    EXPECT_EQ(service_->pacController_.Decide().source, Source::GLOBAL);
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    EXPECT_EQ(Effective().GetHost(), "manual.test");
}

HWTEST_F(NetPacProxyControllerTest, UnresolvedGlobalProxyDoesNotLoadWifi, TestSize.Level1)
{
    SetNetwork(PacProxy("http://test/wifi.pac"));
    service_->globalHttpProxyCache_.EnsureInsert(userId_, HttpProxy());
    EXPECT_EQ(service_->pacController_.Decide().source, Source::NONE);
}

HWTEST_F(NetPacProxyControllerTest, RunningGlobalServerWinsWithEmptyGlobalHost, TestSize.Level1)
{
    SetNetwork(PacProxy("http://test/wifi.pac"));
    auto &controller = service_->pacController_;
    EXPECT_FALSE(controller.IsGlobalPacRunning());
    service_->netPACProxyServer_ = std::make_shared<ProxyServer>(0, 1);
    EXPECT_FALSE(controller.IsGlobalPacRunning());
    ASSERT_TRUE(service_->netPACProxyServer_->Start());
    EXPECT_TRUE(controller.IsGlobalPacRunning());
    controller.ApplyWifiPac();
    EXPECT_EQ(controller.Decide().source, Source::GLOBAL);
    EXPECT_EQ(controller.server_, nullptr);
    EXPECT_TRUE(service_->netPACProxyServer_->IsRunning());
}

HWTEST_F(NetPacProxyControllerTest, HasWifiPacChecksConfigurationAndRuntime, TestSize.Level1)
{
    auto &controller = service_->pacController_;
    EXPECT_FALSE(controller.HasWifiPac());
    SetNetwork(HttpProxy());
    EXPECT_FALSE(controller.HasWifiPac());
    SetNetwork(PacProxy("http://test/wifi.pac"), BEARER_CELLULAR);
    EXPECT_FALSE(controller.HasWifiPac());
    SetNetwork(PacProxy("http://test/wifi.pac"));
    EXPECT_TRUE(controller.HasWifiPac());
    service_->defaultNetSupplier_ = nullptr;
    controller.runtime_.port = LOCAL_PROXY_TEST_PORT;
    EXPECT_TRUE(controller.HasWifiPac());
}

HWTEST_F(NetPacProxyControllerTest, LoadPublishesEndpointWithoutRewritingNetworkOrGlobalConfig, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    auto supplier = SetNetwork(PacProxy(http.Url()));
    EXPECT_TRUE(Effective().GetHost().empty());
    EXPECT_EQ(Effective().GetPort(), 0);
    service_->pacController_.ApplyWifiPac();
    ASSERT_NE(service_->pacController_.server_, nullptr);
    EXPECT_EQ(Effective().GetHost(), "127.0.0.1");
    EXPECT_GT(Effective().GetPort(), 0);
    EXPECT_EQ(Effective().GetPacUrl(), http.Url());
    EXPECT_TRUE(supplier->GetNetwork()->GetHttpProxy().GetHost().empty());
    EXPECT_EQ(supplier->GetNetwork()->GetHttpProxy().GetPort(), 0);
    HttpProxy global;
    service_->LoadGlobalHttpProxy(NetConnService::ACTIVE, global);
    EXPECT_TRUE(global.GetHost().empty());
    EXPECT_EQ(service_->pacController_.server_->pacFunction_("http://example.test", "example.test"),
        "PROXY wifi.test:8080");
}

HWTEST_F(NetPacProxyControllerTest, SameTargetReusesListenerAndScript, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    auto &controller = service_->pacController_;
    controller.ApplyWifiPac();
    auto server = controller.server_;
    ASSERT_NE(server, nullptr);
    int32_t port = controller.runtime_.port;
    controller.ApplyWifiPac();
    EXPECT_EQ(controller.server_, server);
    EXPECT_EQ(controller.runtime_.port, port);
    EXPECT_EQ(http.Requests(), 1);
}

HWTEST_F(NetPacProxyControllerTest, UrlChangeReplacesListenerAndScript, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    auto supplier = SetNetwork(PacProxy(http.Url()));
    auto &controller = service_->pacController_;
    controller.ApplyWifiPac();
    auto original = controller.server_;
    ASSERT_NE(original, nullptr);
    supplier->GetNetwork()->netLinkInfo_.httpProxy_ = PacProxy(http.Url("/other.pac"));
    EXPECT_TRUE(Effective().GetHost().empty());
    controller.ApplyWifiPac();
    EXPECT_NE(controller.server_, original);
    EXPECT_FALSE(original->IsRunning());
    EXPECT_EQ(Effective().GetPacUrl(), http.Url("/other.pac"));
    EXPECT_EQ(http.Requests(), EXPECTED_DOWNLOADS_AFTER_RELOAD);
}

HWTEST_F(NetPacProxyControllerTest, StoppedListenerIsRecreated, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    auto &controller = service_->pacController_;
    controller.ApplyWifiPac();
    ASSERT_NE(controller.server_, nullptr);
    controller.server_->Stop();
    controller.ApplyWifiPac();
    ASSERT_NE(controller.server_, nullptr);
    EXPECT_TRUE(controller.server_->IsRunning());
    EXPECT_EQ(http.Requests(), EXPECTED_DOWNLOADS_AFTER_RELOAD);
}

HWTEST_F(NetPacProxyControllerTest, WifiRemovalStopsListenerAndClearsEndpoint, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    auto &controller = service_->pacController_;
    controller.ApplyWifiPac();
    ASSERT_NE(controller.server_, nullptr);
    SetNetwork(HttpProxy(), BEARER_CELLULAR, OTHER_NET_ID);
    controller.ApplyWifiPac();
    EXPECT_EQ(controller.server_, nullptr);
    EXPECT_EQ(controller.runtime_.port, 0);
    EXPECT_TRUE(Effective().GetHost().empty());
    EXPECT_FALSE(controller.StopServer());
    controller.ApplyWifiPac();
    EXPECT_EQ(controller.server_, nullptr);
}

HWTEST_F(NetPacProxyControllerTest, ManualGlobalTakeoverStopsOnlyWifiListener, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    service_->pacController_.ApplyWifiPac();
    ASSERT_NE(service_->pacController_.server_, nullptr);
    SetGlobalProxy(ManualProxy());
    service_->pacController_.ApplyWifiPac();
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    EXPECT_EQ(Effective().GetHost(), "manual.test");
}

HWTEST_F(NetPacProxyControllerTest, DownloadAndParseFailuresNeverPublishEndpoint, TestSize.Level1)
{
    for (const auto &body : {std::string(), std::string("function FindProxyForURL( {")}) {
        PacHttpServer http(body);
        ASSERT_TRUE(http.Start());
        SetNetwork(PacProxy(http.Url()));
        service_->pacController_.ApplyWifiPac();
        EXPECT_EQ(service_->pacController_.server_, nullptr);
        EXPECT_EQ(Effective().GetPort(), 0);
    }
    PacHttpServer http(WIFI_SCRIPT, HTTP_UNAVAILABLE);
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    service_->pacController_.ApplyWifiPac();
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    EXPECT_EQ(Effective().GetPort(), 0);
}

HWTEST_F(NetPacProxyControllerTest, TargetChangeDuringDownloadDiscardsResult, TestSize.Level1)
{
    PacHttpServer http(WIFI_SCRIPT, HTTP_OK, [this]() {
        std::unique_lock<ffrt::shared_mutex> lock(service_->defaultNetSupplierMutex_);
        service_->defaultNetSupplier_ = nullptr;
    });
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    service_->pacController_.ApplyWifiPac();
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    EXPECT_EQ(Effective().GetPort(), 0);
}

HWTEST_F(NetPacProxyControllerTest, TargetChangeAfterStoppingOldListenerKeepsEndpointCleared, TestSize.Level1)
{
    PacHttpServer first;
    ASSERT_TRUE(first.Start());
    SetNetwork(PacProxy(first.Url()));
    service_->pacController_.ApplyWifiPac();
    ASSERT_NE(service_->pacController_.server_, nullptr);
    PacHttpServer second(WIFI_SCRIPT, HTTP_OK, [this]() {
        std::unique_lock<ffrt::shared_mutex> lock(service_->defaultNetSupplierMutex_);
        service_->defaultNetSupplier_ = nullptr;
    });
    ASSERT_TRUE(second.Start());
    SetNetwork(PacProxy(second.Url()));
    service_->pacController_.ApplyWifiPac();
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    EXPECT_EQ(service_->pacController_.runtime_.port, 0);
}

HWTEST_F(NetPacProxyControllerTest, DownloadDoesNotHoldOperationLockAndGlobalTakeoverWins, TestSize.Level1)
{
    std::atomic<bool> acquired{false};
    PacHttpServer http(WIFI_SCRIPT, HTTP_OK, [this, &acquired]() {
        acquired = service_->pacController_.operationMutex_.try_lock();
        if (acquired) {
            SetGlobalProxy(ManualProxy());
            service_->pacController_.operationMutex_.unlock();
        }
    });
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    service_->pacController_.ApplyWifiPac();
    EXPECT_TRUE(acquired);
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    EXPECT_EQ(Effective().GetHost(), "manual.test");
}

HWTEST_F(NetPacProxyControllerTest, FillRequiresReadyMatchingNetworkUrlAndEmptyHost, TestSize.Level1)
{
    auto &controller = service_->pacController_;
    HttpProxy proxy = PacProxy("http://test/wifi.pac");
    controller.FillLocalProxy(WIFI_NET_ID, proxy);
    EXPECT_TRUE(proxy.GetHost().empty());
    controller.runtime_ = {{Source::WIFI, WIFI_NET_ID, proxy.GetPacUrl()}, LOCAL_PROXY_TEST_PORT};
    controller.FillLocalProxy(OTHER_NET_ID, proxy);
    EXPECT_TRUE(proxy.GetHost().empty());
    proxy.SetPacUrl("http://test/other.pac");
    controller.FillLocalProxy(WIFI_NET_ID, proxy);
    EXPECT_TRUE(proxy.GetHost().empty());
    proxy.SetPacUrl("http://test/wifi.pac");
    proxy.SetHost("wifi.manual");
    controller.FillLocalProxy(WIFI_NET_ID, proxy);
    EXPECT_EQ(proxy.GetHost(), "wifi.manual");
    proxy.SetHost("");
    controller.FillLocalProxy(WIFI_NET_ID, proxy);
    EXPECT_EQ(proxy.GetHost(), "127.0.0.1");
    EXPECT_EQ(proxy.GetPort(), LOCAL_PROXY_TEST_PORT);
}

HWTEST_F(NetPacProxyControllerTest, GetDefaultProxyFillsOnlyDefaultBoundNetwork, TestSize.Level1)
{
    auto other = SetNetwork(PacProxy("http://test/wifi.pac"), BEARER_WIFI, OTHER_NET_ID);
    auto wifi = SetNetwork(PacProxy("http://test/wifi.pac"));
    service_->pacController_.runtime_ = {{Source::WIFI, WIFI_NET_ID, "http://test/wifi.pac"}, LOCAL_PROXY_TEST_PORT};
    EXPECT_EQ(Effective(WIFI_NET_ID).GetPort(), LOCAL_PROXY_TEST_PORT);
    EXPECT_EQ(Effective().GetPort(), LOCAL_PROXY_TEST_PORT);
    EXPECT_TRUE(Effective(OTHER_NET_ID).GetHost().empty());
    other->GetNetwork()->state_ = NET_CONN_STATE_DISCONNECTED;
    EXPECT_EQ(Effective(OTHER_NET_ID).GetPort(), LOCAL_PROXY_TEST_PORT);
    service_->defaultNetSupplier_ = nullptr;
    EXPECT_TRUE(Effective(WIFI_NET_ID).GetHost().empty());
    EXPECT_TRUE(Effective().GetHost().empty());
}

HWTEST_F(NetPacProxyControllerTest, PauseStopsWifiAndResumesAfterScope, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    auto &controller = service_->pacController_;
    controller.ApplyWifiPac();
    ASSERT_NE(controller.server_, nullptr);
    {
        auto pause = controller.PauseWifiPac();
        EXPECT_EQ(controller.server_, nullptr);
        EXPECT_EQ(Effective().GetPort(), 0);
    }
    WaitQueue();
    ASSERT_NE(controller.server_, nullptr);
    EXPECT_GT(Effective().GetPort(), 0);
}

HWTEST_F(NetPacProxyControllerTest, PauseWithoutWifiStillSchedulesRefresh, TestSize.Level1)
{
    {
        auto pause = service_->pacController_.PauseWifiPac();
        EXPECT_EQ(service_->pacController_.server_, nullptr);
    }
    ASSERT_NE(service_->pacController_.queue_, nullptr);
    WaitQueue();
    EXPECT_EQ(service_->pacController_.runtime_.port, 0);
}

HWTEST_F(NetPacProxyControllerTest, GlobalOffWithAlreadyEmptyHostStartsWifi, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    service_->netPACProxyServer_ = std::make_shared<ProxyServer>(0, 1);
    ASSERT_TRUE(service_->netPACProxyServer_->Start());
    service_->pacController_.ApplyWifiPac();
    EXPECT_EQ(service_->pacController_.server_, nullptr);
    ASSERT_EQ(service_->SetProxyOff(), NETMANAGER_SUCCESS);
    WaitQueue();
    EXPECT_FALSE(service_->netPACProxyServer_->IsRunning());
    EXPECT_GT(Effective().GetPort(), 0);
}

HWTEST_F(NetPacProxyControllerTest, RepeatedGlobalOffPreservesWorkingWifi, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    SetNetwork(PacProxy(http.Url()));
    service_->pacController_.ApplyWifiPac();
    auto server = service_->pacController_.server_;
    ASSERT_NE(server, nullptr);
    ASSERT_EQ(service_->SetProxyOff(), NETMANAGER_SUCCESS);
    WaitQueue();
    EXPECT_EQ(service_->pacController_.server_, server);
    EXPECT_TRUE(server->IsRunning());
    EXPECT_EQ(http.Requests(), 1);
}

HWTEST_F(NetPacProxyControllerTest, RefreshUsesLatestConfigurationOnSerialQueue, TestSize.Level1)
{
    auto &controller = service_->pacController_;
    controller.Refresh();
    ASSERT_NE(controller.queue_, nullptr);
    WaitQueue();
    std::promise<void> release;
    auto gate = release.get_future().share();
    controller.queue_->submit_h([gate]() { gate.wait(); });
    PacHttpServer http;
    bool listening = http.Start();
    SetNetwork(PacProxy(http.Url("/first.pac")));
    controller.Refresh();
    SetNetwork(PacProxy(http.Url("/latest.pac")));
    controller.Refresh();
    release.set_value();
    ASSERT_TRUE(listening);
    WaitQueue();
    EXPECT_EQ(Effective().GetPacUrl(), http.Url("/latest.pac"));
    EXPECT_EQ(http.Requests(), 1);
}

HWTEST_F(NetPacProxyControllerTest, RefreshWithoutSharedOwnerDoesNotRunTask, TestSize.Level1)
{
    NetConnService unowned;
    unowned.pacController_.Refresh();
    auto queue = unowned.pacController_.queue_;
    ASSERT_NE(queue, nullptr);
    auto barrier = queue->submit_h([]() {});
    queue->wait(barrier);
    EXPECT_EQ(unowned.pacController_.server_, nullptr);
    EXPECT_EQ(unowned.pacController_.runtime_.port, 0);
}

HWTEST_F(NetPacProxyControllerTest, LinkChangesOnlyRefreshWhenProxyChanges, TestSize.Level1)
{
    auto &controller = service_->pacController_;
    service_->HandleNetLinkHttpProxyChange(HttpProxy(), HttpProxy());
    service_->HandleNetLinkHttpProxyChange(HttpProxy(), ManualProxy());
    EXPECT_EQ(controller.queue_, nullptr);
    auto pac = PacProxy("http://test/wifi.pac");
    service_->HandleNetLinkHttpProxyChange(pac, pac);
    EXPECT_EQ(controller.queue_, nullptr);
    service_->HandleNetLinkHttpProxyChange(pac, HttpProxy());
    ASSERT_NE(controller.queue_, nullptr);
    WaitQueue();
    SetGlobalProxy(ManualProxy());
    SetNetwork(pac);
    service_->HandleNetLinkHttpProxyChange(HttpProxy(), pac);
    service_->HandleNetLinkHttpProxyChange(ManualProxy("old"), ManualProxy("new"));
    WaitQueue();
    EXPECT_EQ(controller.server_, nullptr);
    EXPECT_EQ(Effective().GetHost(), "manual.test");
}

HWTEST_F(NetPacProxyControllerTest, StaticDownloadHandlesHttpStatusEmptyAndValidBody, TestSize.Level1)
{
    for (const auto &body : {std::string(WIFI_SCRIPT), std::string("var plainScript = 1;"), std::string()}) {
        PacHttpServer http(body);
        ASSERT_TRUE(http.Start());
        std::string downloaded;
        EXPECT_EQ(NetPACManager::DownloadPACScript(http.Url(), downloaded), !body.empty());
        EXPECT_EQ(downloaded, body);
    }
    PacHttpServer error(WIFI_SCRIPT, HTTP_NOT_FOUND);
    ASSERT_TRUE(error.Start());
    std::string downloaded;
    EXPECT_FALSE(NetPACManager::DownloadPACScript(error.Url(), downloaded));
    downloaded.clear();
    EXPECT_FALSE(NetPACManager::DownloadPACScript("", downloaded));
}

HWTEST_F(NetPacProxyControllerTest, DownloadDoesNotReplaceAlreadyLoadedScript, TestSize.Level1)
{
    auto manager = service_->GetNetPacManager();
    ASSERT_TRUE(manager->LoadPACScript("function FindProxyForURL(u, h) { return 'DIRECT'; }"));
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    std::string downloaded;
    ASSERT_TRUE(NetPACManager::DownloadPACScript(http.Url(), downloaded));
    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://test", "test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "DIRECT");
}

HWTEST_F(NetPacProxyControllerTest, WifiLoadPreservesSavedGlobalPacUrl, TestSize.Level1)
{
    PacHttpServer http;
    ASSERT_TRUE(http.Start());
    auto manager = service_->GetNetPacManager();
    const std::string globalUrl = http.Url("/global.pac");
    ASSERT_TRUE(manager->InitPACScriptWithURL(globalUrl));
    ASSERT_TRUE(manager->LoadPACScript("function FindProxyForURL(u, h) { return 'DIRECT'; }"));
    EXPECT_EQ(manager->scriptFileUrl_, globalUrl);
    std::string proxy;
    EXPECT_EQ(manager->FindProxyForURL("http://test", "test", proxy), PAC_OK);
    EXPECT_EQ(proxy, "DIRECT");
    EXPECT_EQ(http.Requests(), 1);
}
#endif // NETMANAGER_ENABLE_PAC_PROXY
} // namespace NetManagerStandard
} // namespace OHOS
