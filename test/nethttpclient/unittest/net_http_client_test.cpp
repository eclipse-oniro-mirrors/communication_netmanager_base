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

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>

#include "mock_http_server.h"
#include "http_easy_handle.h"
#include "net_http_client.h"

using namespace testing::ext;

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {
namespace {
constexpr int32_t TEST_TIMEOUT_MS = 3000;
constexpr int32_t SHORT_TIMEOUT_MS = 300;

const char *RESP_200_JSON =
    "HTTP/1.1 200 OK\r\n"
    "Server: mock\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "{\"code\":\"ok\"}";

const char *RESP_204_NO_CONTENT =
    "HTTP/1.1 204 No Content\r\n"
    "X-Req-Id: abc123\r\n"
    "\r\n";

const char *RESP_302_REDIRECT =
    "HTTP/1.1 302 Found\r\n"
    "Location: http://127.0.0.1:1/portal\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

const char *RESP_404 =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Length: 9\r\n"
    "\r\n"
    "not found";

/* Callback synchronization helper. */
class CallbackLatch {
public:
    void Notify(const HttpResponse &resp)
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            response = resp;
            fired = true;
        }
        cv.notify_one();
    }

    bool Wait(int32_t timeoutMs)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return fired; });
    }

    HttpResponse response;
    bool fired = false;

private:
    std::mutex mtx_;
    std::condition_variable cv;
};
} // namespace

class NetHttpClientTest : public testing::Test {
protected:
    NetHttpClient &client_ = NetHttpClient::GetInstance();
};

/*
 * Sync: normal GET, transport success with response code / body / headers.
 */
HWTEST_F(NetHttpClientTest, ExecuteGetSuccess, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/path";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_TRUE(response.IsTransportSuccess());
    EXPECT_EQ(response.responseCode, 200);
    EXPECT_EQ(response.body, "{\"code\":\"ok\"}");
    EXPECT_NE(response.headers.find("content-type"), response.headers.end());
    EXPECT_NE(response.headers.find("content-length"), response.headers.end());
    EXPECT_EQ(response.headers["server"], "mock");
    EXPECT_GE(response.totalCostMs, 0);
    EXPECT_EQ(response.rawCurlCode, 0);
}

/*
 * Sync: custom headers must reach the server as "key: value" lines.
 */
HWTEST_F(NetHttpClientTest, ExecuteGetCustomHeaders, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/hdr";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.headers["Connection"] = "close";
    request.headers["User-Agent"] = "UnitTestAgent";
    HttpResponse response = client_.Execute(request);
    ASSERT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);

    std::string raw = server.LastRequest();
    EXPECT_NE(raw.find("GET /hdr HTTP/1.1"), std::string::npos);
    EXPECT_NE(raw.find("Connection: close"), std::string::npos);
    EXPECT_NE(raw.find("User-Agent: UnitTestAgent"), std::string::npos);
    /* Note: empty-valued custom headers ("X-Empty") are dropped by some curl
     * versions, so they are intentionally not asserted here. */
}

/*
 * Sync: POST body must be sent, method is POST.
 */
HWTEST_F(NetHttpClientTest, ExecutePostBody, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/submit";
    request.method = HttpMethod::POST;
    request.body = "hello=post&body=1";
    request.headers["Content-Type"] = "application/x-www-form-urlencoded";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    std::string raw = server.LastRequest();
    EXPECT_NE(raw.find("POST /submit HTTP/1.1"), std::string::npos);
    EXPECT_NE(raw.find(request.body), std::string::npos);
}

/*
 * Sync: 4xx is a transport success, business judgement is up to the caller.
 */
HWTEST_F(NetHttpClientTest, Execute404StillTransportSuccess, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_404));

    HttpRequest request;
    request.url = server.BaseUrl() + "/missing";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 404);
    EXPECT_EQ(response.body, "not found");
}

/*
 * Sync: redirect is not followed by default, redirectUrl is exposed.
 */
HWTEST_F(NetHttpClientTest, ExecuteRedirectNotFollowed, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_302_REDIRECT));

    HttpRequest request;
    request.url = server.BaseUrl() + "/redir";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 302);
    EXPECT_EQ(response.redirectUrl, "http://127.0.0.1:1/portal");
    EXPECT_EQ(response.headers["location"], "http://127.0.0.1:1/portal");
}

/*
 * Sync: 204 with empty body and duplicate headers.
 */
HWTEST_F(NetHttpClientTest, Execute204NoContent, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_204_NO_CONTENT));

    HttpRequest request;
    request.url = server.BaseUrl() + "/probe";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.forbidReuse = true;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 204);
    EXPECT_TRUE(response.body.empty());
    EXPECT_EQ(response.headers["x-req-id"], "abc123");
}

/*
 * Sync: invalid url yields INVALID_PARAM without touching the network.
 */
HWTEST_F(NetHttpClientTest, ExecuteInvalidUrl, TestSize.Level0)
{
    HttpRequest request;
    request.url = "";
    HttpResponse response = client_.Execute(request);
    EXPECT_EQ(response.errCode, NetHttpErrorCode::INVALID_PARAM);
    EXPECT_EQ(response.responseCode, -1);
}

/*
 * Sync: connection refused maps to CONNECT_FAILED.
 */
HWTEST_F(NetHttpClientTest, ExecuteConnectRefused, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));
    uint16_t port = server.Port();
    server.Stop(); /* The port is now closed, connect will be refused. */

    HttpRequest request;
    request.url = "http://127.0.0.1:" + std::to_string(port) + "/refused";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.connectTimeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_NE(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, -1);
    EXPECT_FALSE(response.errMessage.empty());
}

/*
 * Sync: a server that never replies triggers OPERATION_TIMEDOUT.
 */
HWTEST_F(NetHttpClientTest, ExecuteOperationTimeout, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::HANG));

    HttpRequest request;
    request.url = server.BaseUrl() + "/hang";
    request.timeoutMs = SHORT_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::OPERATION_TIMEDOUT);
    EXPECT_EQ(response.rawCurlCode, 28); /* CURLE_OPERATION_TIMEDOUT */
    EXPECT_GE(response.totalCostMs, SHORT_TIMEOUT_MS - 50);
}

/*
 * Async: submit a GET, callback fires on the engine thread with the response.
 */
HWTEST_F(NetHttpClientTest, SubmitGetSuccess, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    CallbackLatch latch;
    HttpRequest request;
    request.url = server.BaseUrl() + "/async";
    request.timeoutMs = TEST_TIMEOUT_MS;
    auto task = client_.Submit(request, [&latch](const HttpResponse &response) { latch.Notify(response); });
    ASSERT_NE(task, nullptr);

    ASSERT_TRUE(latch.Wait(TEST_TIMEOUT_MS));
    EXPECT_EQ(latch.response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(latch.response.responseCode, 200);
    EXPECT_EQ(latch.response.body, "{\"code\":\"ok\"}");
    EXPECT_TRUE(task->IsDone());
    EXPECT_GT(task->TaskId(), 0U);
}

/*
 * Async: cancel an in-flight task, callback must fire with CANCELLED.
 */
HWTEST_F(NetHttpClientTest, SubmitThenCancel, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::HANG));

    CallbackLatch latch;
    HttpRequest request;
    request.url = server.BaseUrl() + "/cancel-me";
    request.timeoutMs = TEST_TIMEOUT_MS;
    auto task = client_.Submit(request, [&latch](const HttpResponse &response) { latch.Notify(response); });
    ASSERT_NE(task, nullptr);
    EXPECT_FALSE(task->IsDone());

    task->Cancel();
    ASSERT_TRUE(latch.Wait(TEST_TIMEOUT_MS));
    EXPECT_EQ(latch.response.errCode, NetHttpErrorCode::CANCELLED);
    EXPECT_TRUE(task->IsDone());
}

/*
 * Async: cancel after completion is a no-op and does not change the result.
 */
HWTEST_F(NetHttpClientTest, CancelAfterDone, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    CallbackLatch latch;
    HttpRequest request;
    request.url = server.BaseUrl() + "/done";
    request.timeoutMs = TEST_TIMEOUT_MS;
    auto task = client_.Submit(request, [&latch](const HttpResponse &response) { latch.Notify(response); });
    ASSERT_NE(task, nullptr);
    ASSERT_TRUE(latch.Wait(TEST_TIMEOUT_MS));
    ASSERT_TRUE(task->IsDone());

    task->Cancel();
    EXPECT_EQ(latch.response.errCode, NetHttpErrorCode::SUCCESS);
}

/*
 * Async: engine thread exits when idle and is transparently restarted.
 */
HWTEST_F(NetHttpClientTest, EngineRestartAfterIdle, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    CallbackLatch first;
    HttpRequest request;
    request.url = server.BaseUrl() + "/first";
    request.timeoutMs = TEST_TIMEOUT_MS;
    auto task = client_.Submit(request, [&first](const HttpResponse &response) { first.Notify(response); });
    ASSERT_NE(task, nullptr);
    ASSERT_TRUE(first.Wait(TEST_TIMEOUT_MS));

    /* Give the engine thread time to notice the idle state and exit. */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CallbackLatch second;
    request.url = server.BaseUrl() + "/second";
    auto task2 = client_.Submit(request, [&second](const HttpResponse &response) { second.Notify(response); });
    ASSERT_NE(task2, nullptr);
    ASSERT_TRUE(second.Wait(TEST_TIMEOUT_MS));
    EXPECT_EQ(second.response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(second.response.responseCode, 200);
}

/*
 * Async: multiple concurrent submits are all driven by one engine run.
 */
HWTEST_F(NetHttpClientTest, SubmitConcurrent, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    constexpr int TASK_COUNT = 4;
    CallbackLatch latches[TASK_COUNT];
    HttpRequest request;
    request.url = server.BaseUrl() + "/multi";
    request.timeoutMs = TEST_TIMEOUT_MS;
    for (int i = 0; i < TASK_COUNT; i++) {
        auto task = client_.Submit(request,
                                   [&latches, i](const HttpResponse &response) { latches[i].Notify(response); });
        ASSERT_NE(task, nullptr);
    }
    for (int i = 0; i < TASK_COUNT; i++) {
        ASSERT_TRUE(latches[i].Wait(TEST_TIMEOUT_MS));
        EXPECT_EQ(latches[i].response.errCode, NetHttpErrorCode::SUCCESS);
        EXPECT_EQ(latches[i].response.responseCode, 200);
    }
}

/*
 * Async: invalid url returns nullptr without any callback.
 */
HWTEST_F(NetHttpClientTest, SubmitInvalidUrl, TestSize.Level0)
{
    HttpRequest request;
    request.url = "";
    auto task = client_.Submit(request, [](const HttpResponse &) {});
    EXPECT_EQ(task, nullptr);
}

/*
 * Async: timeout error is reported through the callback.
 */
HWTEST_F(NetHttpClientTest, SubmitTimeout, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::HANG));

    CallbackLatch latch;
    HttpRequest request;
    request.url = server.BaseUrl() + "/hang";
    request.timeoutMs = SHORT_TIMEOUT_MS;
    auto task = client_.Submit(request, [&latch](const HttpResponse &response) { latch.Notify(response); });
    ASSERT_NE(task, nullptr);
    ASSERT_TRUE(latch.Wait(TEST_TIMEOUT_MS + 500));
    EXPECT_EQ(latch.response.errCode, NetHttpErrorCode::OPERATION_TIMEDOUT);
    EXPECT_TRUE(task->IsDone());
}

/*
 * Sync: followRedirects follows a 302 chain to the final response.
 */
HWTEST_F(NetHttpClientTest, ExecuteRedirectFollowed, TestSize.Level0)
{
    MockHttpServer finalServer;
    ASSERT_TRUE(finalServer.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));
    std::string redirect = "HTTP/1.1 302 Found\r\nLocation: " + finalServer.BaseUrl() +
        "/final\r\nContent-Length: 0\r\n\r\n";
    MockHttpServer redirectServer;
    ASSERT_TRUE(redirectServer.Start(MockHttpServer::Behavior::RESPONSE, redirect));

    HttpRequest request;
    request.url = redirectServer.BaseUrl() + "/redir";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.followRedirects = true;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
    EXPECT_EQ(response.body, "{\"code\":\"ok\"}");
    EXPECT_NE(response.effectiveUrl.find("/final"), std::string::npos);
}

/*
 * Sync: a response body above the 10MB limit is aborted on purpose with
 * RESPONSE_TOO_LARGE (CURLE_WRITE_ERROR from the write callback).
 */
HWTEST_F(NetHttpClientTest, ExecuteResponseBodyTooLarge, TestSize.Level0)
{
    std::string body(10 * 1024 * 1024 + 1024, 'x');
    std::string raw = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, raw));

    HttpRequest request;
    request.url = server.BaseUrl() + "/large";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::RESPONSE_TOO_LARGE);
    EXPECT_EQ(response.rawCurlCode, 23); /* CURLE_WRITE_ERROR */
    EXPECT_EQ(response.responseCode, -1);
}

/*
 * Sync: a host that cannot be resolved maps to RESOLVE_FAILED.
 * ".invalid" is reserved by RFC 6761 and never resolves.
 * On a device whose DNS blackholes such queries, curl hits the transfer
 * timeout first, so OPERATION_TIMEDOUT is accepted as an offline-device
 * outcome. The mapping itself is covered hermetically by MapCurlCodeTest.
 */
HWTEST_F(NetHttpClientTest, ExecuteResolveFailed, TestSize.Level0)
{
    HttpRequest request;
    request.url = "http://resolution-definitely-fails.invalid/";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_TRUE(response.errCode == NetHttpErrorCode::RESOLVE_FAILED ||
                response.errCode == NetHttpErrorCode::OPERATION_TIMEDOUT)
        << "errCode=" << static_cast<int32_t>(response.errCode);
    EXPECT_FALSE(response.errMessage.empty());
}

/*
 * Unit: CURLcode -> NetHttpErrorCode translation, pure mapping without any
 * network or DNS dependency (mock-free hermetic coverage of the contract).
 */
HWTEST_F(NetHttpClientTest, MapCurlCodeTest, TestSize.Level0)
{
    HttpEasyHandle handle;
    EXPECT_EQ(handle.MapCurlCode(CURLE_OK), NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(handle.MapCurlCode(CURLE_COULDNT_RESOLVE_HOST), NetHttpErrorCode::RESOLVE_FAILED);
    EXPECT_EQ(handle.MapCurlCode(CURLE_COULDNT_RESOLVE_PROXY), NetHttpErrorCode::RESOLVE_FAILED);
    EXPECT_EQ(handle.MapCurlCode(CURLE_COULDNT_CONNECT), NetHttpErrorCode::CONNECT_FAILED);
    EXPECT_EQ(handle.MapCurlCode(CURLE_OPERATION_TIMEDOUT), NetHttpErrorCode::OPERATION_TIMEDOUT);
    EXPECT_EQ(handle.MapCurlCode(CURLE_WRITE_ERROR), NetHttpErrorCode::RESPONSE_TOO_LARGE);
    EXPECT_EQ(handle.MapCurlCode(CURLE_ABORTED_BY_CALLBACK), NetHttpErrorCode::CANCELLED);
    EXPECT_EQ(handle.MapCurlCode(CURLE_FAILED_INIT), NetHttpErrorCode::INIT_FAILED);
}

/*
 * Sync: resolves entries inject dns results, empty entries are skipped.
 */
HWTEST_F(NetHttpClientTest, ExecuteResolveHostInjection, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = "http://mock.local:" + std::to_string(server.Port()) + "/injected";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.resolves = { "mock.local:" + std::to_string(server.Port()) + ":127.0.0.1", "" };
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Sync: an explicit port overrides the port carried (or defaulted) by the url.
 */
HWTEST_F(NetHttpClientTest, ExecutePortOverride, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = "http://127.0.0.1/override"; /* no port: would default to 80 */
    request.port = server.Port();
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Async: a callback may submit again (re-entrancy contract, no deadlock).
 */
HWTEST_F(NetHttpClientTest, SubmitCallbackReSubmit, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    CallbackLatch second;
    HttpRequest request;
    request.url = server.BaseUrl() + "/first";
    request.timeoutMs = TEST_TIMEOUT_MS;
    auto task = client_.Submit(request, [this, &second, &request, &server](const HttpResponse &response) {
        EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
        HttpRequest next = request;
        next.url = server.BaseUrl() + "/second";
        auto nested = client_.Submit(next, [&second](const HttpResponse &resp) { second.Notify(resp); });
        EXPECT_NE(nested, nullptr);
    });
    ASSERT_NE(task, nullptr);

    ASSERT_TRUE(second.Wait(2 * TEST_TIMEOUT_MS));
    EXPECT_EQ(second.response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(second.response.responseCode, 200);
    EXPECT_TRUE(task->IsDone());
}

/*
 * Sync: PUT with a body sends method PUT and the body.
 */
HWTEST_F(NetHttpClientTest, ExecutePutBody, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/put";
    request.method = HttpMethod::PUT;
    request.body = "put-payload";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    std::string raw = server.LastRequest();
    EXPECT_NE(raw.find("PUT /put HTTP/1.1"), std::string::npos);
    EXPECT_NE(raw.find("put-payload"), std::string::npos);
}

/*
 * Sync: PUT without a body sends method PUT and no body.
 */
HWTEST_F(NetHttpClientTest, ExecutePutWithoutBody, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/put-empty";
    request.method = HttpMethod::PUT;
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_NE(server.LastRequest().find("PUT /put-empty HTTP/1.1"), std::string::npos);
}

/*
 * Sync: DELETE with a body sends method DELETE and the body.
 */
HWTEST_F(NetHttpClientTest, ExecuteDeleteWithBody, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/delete";
    request.method = HttpMethod::DELETE;
    request.body = "delete-payload";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    std::string raw = server.LastRequest();
    EXPECT_NE(raw.find("DELETE /delete HTTP/1.1"), std::string::npos);
    EXPECT_NE(raw.find("delete-payload"), std::string::npos);
}

/*
 * Sync: DELETE without a body sends method DELETE and no body.
 */
HWTEST_F(NetHttpClientTest, ExecuteDeleteWithoutBody, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/delete-empty";
    request.method = HttpMethod::DELETE;
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_NE(server.LastRequest().find("DELETE /delete-empty HTTP/1.1"), std::string::npos);
}

/*
 * Sync: HEAD sends method HEAD and ignores the response body.
 */
HWTEST_F(NetHttpClientTest, ExecuteHeadMethod, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/head";
    request.method = HttpMethod::HEAD;
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
    EXPECT_NE(server.LastRequest().find("HEAD /head HTTP/1.1"), std::string::npos);
}

/*
 * Sync: a plain http proxy receives the absolute-uri request form.
 */
HWTEST_F(NetHttpClientTest, ExecuteViaHttpProxy, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = "http://proxy-target.test/path";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpProxyConfig proxy;
    proxy.host = "127.0.0.1";
    proxy.port = server.Port();
    proxy.useTunnel = false; /* classic absolute-uri proxying, no CONNECT */
    request.proxy = proxy;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
    EXPECT_NE(server.LastRequest().find("GET http://proxy-target.test/path HTTP/1.1"), std::string::npos);
}

/*
 * Sync: proxy credentials are applied (auth setopt branch), unused without a 407 challenge.
 */
HWTEST_F(NetHttpClientTest, ExecuteViaProxyWithAuth, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = "http://proxy-target.test/auth";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpProxyConfig proxy;
    proxy.host = "127.0.0.1";
    proxy.port = server.Port();
    proxy.useTunnel = false;
    proxy.username = "unit-user";
    proxy.password = "unit-pass";
    request.proxy = proxy;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Sync: a proxy entry with an empty host is ignored, the request goes direct.
 */
HWTEST_F(NetHttpClientTest, ExecuteProxyEmptyHostIgnored, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/direct";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.proxy = HttpProxyConfig {}; /* host empty: early-ignored */
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Sync: every tls option branch executes and stays inert for plain http.
 */
HWTEST_F(NetHttpClientTest, ExecuteTlsOptionBranches, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/tls-options";
    request.timeoutMs = TEST_TIMEOUT_MS;
    request.tls.verifyPeer = false;
    request.tls.caInfoPath = "/data/local/tmp/nonexistent-ca.pem";
    request.tls.caDirPath = "/data/local/tmp/nonexistent-ca-dir";
    request.tls.minVersion = TlsVersion::TLS_V1_2;
    request.tls.cipherList = "AES128-SHA256";
    request.tls.tls13CipherList = "TLS_AES_128_GCM_SHA256";
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Sync: IPV4_ONLY reaches the loopback (v4) mock server.
 */
HWTEST_F(NetHttpClientTest, ExecuteIpV4Only, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/v4";
    request.ipVersion = IpVersion::IPV4_ONLY;
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Sync: IPV6_ONLY cannot reach a v4 loopback address, transport fails.
 */
HWTEST_F(NetHttpClientTest, ExecuteIpV6OnlyUnreachable, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/v6"; /* 127.0.0.1 is v4 only */
    request.ipVersion = IpVersion::IPV6_ONLY;
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_NE(response.errCode, NetHttpErrorCode::SUCCESS);
}

/*
 * Sync: the outgoing interface can be bound ("lo" still reaches the loopback server).
 */
HWTEST_F(NetHttpClientTest, ExecuteBindInterfaceLo, TestSize.Level0)
{
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, RESP_200_JSON));

    HttpRequest request;
    request.url = server.BaseUrl() + "/bound";
    request.bindInterface = "lo";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.responseCode, 200);
}

/*
 * Sync: duplicate response headers are joined with ", " and keys are lower-cased.
 */
HWTEST_F(NetHttpClientTest, ExecuteDuplicateHeadersMerged, TestSize.Level0)
{
    const std::string dupResponse =
        "HTTP/1.1 200 OK\r\n"
        "X-Dup: first\r\n"
        "X-Dup: second\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "ok";
    MockHttpServer server;
    ASSERT_TRUE(server.Start(MockHttpServer::Behavior::RESPONSE, dupResponse));

    HttpRequest request;
    request.url = server.BaseUrl() + "/dup";
    request.timeoutMs = TEST_TIMEOUT_MS;
    HttpResponse response = client_.Execute(request);

    EXPECT_EQ(response.errCode, NetHttpErrorCode::SUCCESS);
    EXPECT_EQ(response.headers["x-dup"], "first, second");
}
} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS
