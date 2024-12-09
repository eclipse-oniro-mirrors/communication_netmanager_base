/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <fstream>
#include <sstream>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "dns_quality_diag.h"
#include "third_party/musl/include/netdb.h"
#include "net_handle.h"
#include "net_conn_client.h"

namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
const uint32_t MAX_RESULT_SIZE = 32;
}  // namespace

class DnsQualityDiagTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    NetsysNative::NetDnsResultReport report;
    DnsQualityDiag dnsQualityDiag;
    struct AddrInfo addrinfoIpv4;
    struct AddrInfo addrinfoIpv6;
};

void DnsQualityDiagTest::SetUpTestCase() {}

void DnsQualityDiagTest::TearDownTestCase() {}

void DnsQualityDiagTest::SetUp() {}

void DnsQualityDiagTest::TearDown() {}

HWTEST_F(DnsQualityDiagTest, DnsQualityDiag_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    EXPECT_EQ(dnsQualityDiag.InitHandler(), 0);
}

HWTEST_F(DnsQualityDiagTest, GetInstanceShouldReturnSingletonInstance, TestSize.Level0)
{
    DnsQualityDiag &instance1 = DnsQualityDiag::GetInstance();
    DnsQualityDiag &instance2 = DnsQualityDiag::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

HWTEST_F(DnsQualityDiagTest, SendHealthReport_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    NetsysNative::NetDnsHealthReport healthreport;
    EXPECT_EQ(dnsQualityDiag.SendHealthReport(healthreport), 0);
}

HWTEST_F(DnsQualityDiagTest, ParseReportAddr_ShouldAddIPv4AndIPv6_WhenCalledWithValidAddrInfo, TestSize.Level0)
{
    uint32_t size = 2;
    addrinfoIpv4.aiFamily = AF_INET;
    addrinfoIpv6.aiFamily = AF_INET6;
    struct AddrInfo addrinfo[2] = { addrinfoIpv4, addrinfoIpv6 };

    int32_t returnCode = dnsQualityDiag.ParseReportAddr(size, addrinfo, report);

    EXPECT_EQ(report.addrlist_.size(), 2);
    EXPECT_EQ(returnCode, 0);
}

HWTEST_F(DnsQualityDiagTest, ParseReportAddr_ShouldNotAddMoreThanMaxSize_WhenCalledWithMoreAddrInfo, TestSize.Level0)
{
    uint32_t size = MAX_RESULT_SIZE + 1;
    addrinfoIpv4.aiFamily = AF_INET;
    struct AddrInfo addrinfo[MAX_RESULT_SIZE + 1] = { addrinfoIpv4 };

    int32_t returnCode = dnsQualityDiag.ParseReportAddr(size, addrinfo, report);

    EXPECT_EQ(report.addrlist_.size(), MAX_RESULT_SIZE);
    EXPECT_EQ(returnCode, 0);
}

HWTEST_F(DnsQualityDiagTest, ReportDnsResult_ShouldReturnZero_WhenCalled_01, TestSize.Level0)
{
    uint16_t netId = 1;
    uint16_t uid = 1;
    uint32_t pid = 1;
    int32_t usedtime = 1;
    char name[] = "test";
    uint32_t size = 1;
    int32_t failreason = 0;
    QueryParam queryParam;
    AddrInfo addrinfo;
    EXPECT_EQ(dnsQualityDiag.ReportDnsResult(netId, uid, pid, usedtime, name, size, failreason, queryParam, &addrinfo),
              0);
}

HWTEST_F(DnsQualityDiagTest, ReportDnsResult_ShouldReturnZero_WhenCalled_02, TestSize.Level0)
{
    uint16_t netId = 1;
    uint16_t uid = 1;
    uint32_t pid = 1;
    int32_t usedtime = 1;
    char name[] = "test";
    uint32_t size = 1;
    int32_t failreason = 1;
    QueryParam queryParam;
    AddrInfo addrinfo;
    EXPECT_EQ(dnsQualityDiag.ReportDnsResult(netId, uid, pid, usedtime, name, size, failreason, queryParam, &addrinfo),
              0);
}

HWTEST_F(DnsQualityDiagTest, ReportDnsResult_ShouldIgnore_WhenQueryTypeIsOne, TestSize.Level0)
{
    uint16_t netId = 1;
    uint16_t uid = 1;
    uint32_t pid = 1;
    int32_t usedtime = 100;
    char name[] = "test";
    uint32_t size = 10;
    int32_t failreason = 0;
    QueryParam queryParam;
    queryParam.type = 1;
    AddrInfo addrinfo;

    int32_t result =
        dnsQualityDiag.ReportDnsResult(netId, uid, pid, usedtime, name, size, failreason, queryParam, &addrinfo);
    EXPECT_EQ(result, 0);
}

HWTEST_F(DnsQualityDiagTest, RegisterResultListener_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    sptr<NetsysNative::INetDnsResultCallback> callback;
    uint32_t timeStep = 1;
    EXPECT_EQ(dnsQualityDiag.RegisterResultListener(callback, timeStep), 0);
}

HWTEST_F(DnsQualityDiagTest, UnregisterResultListener_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    sptr<NetsysNative::INetDnsResultCallback> callback;
    EXPECT_EQ(dnsQualityDiag.UnregisterResultListener(callback), 0);
}

HWTEST_F(DnsQualityDiagTest, RegisterHealthListener_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    sptr<NetsysNative::INetDnsHealthCallback> callback;
    EXPECT_EQ(dnsQualityDiag.RegisterHealthListener(callback), 0);
}

HWTEST_F(DnsQualityDiagTest, UnregisterHealthListener_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    sptr<NetsysNative::INetDnsHealthCallback> callback;
    EXPECT_EQ(dnsQualityDiag.UnregisterHealthListener(callback), 0);
}

HWTEST_F(DnsQualityDiagTest, SetLoopDelay_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    int32_t delay = 1;
    EXPECT_EQ(dnsQualityDiag.SetLoopDelay(delay), 0);
}

HWTEST_F(DnsQualityDiagTest, query_default_host_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    EXPECT_EQ(dnsQualityDiag.query_default_host(), 0);
}

HWTEST_F(DnsQualityDiagTest, handle_dns_loop_ShouldReturnZero_WhenCalled_01, TestSize.Level0)
{
    dnsQualityDiag.handler_started = false;
    EXPECT_EQ(dnsQualityDiag.handle_dns_loop(), 0);
}

HWTEST_F(DnsQualityDiagTest, handle_dns_loop_ShouldReturnZero_WhenCalled_02, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    EXPECT_EQ(dnsQualityDiag.handle_dns_loop(), 0);
}

HWTEST_F(DnsQualityDiagTest, handle_dns_fail_ShouldReturnZero_WhenCalled_01, TestSize.Level0)
{
    dnsQualityDiag.handler_started = false;
    EXPECT_EQ(dnsQualityDiag.handle_dns_fail(), 0);
}

HWTEST_F(DnsQualityDiagTest, handle_dns_fail_ShouldReturnZero_WhenCalled_02, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    EXPECT_EQ(dnsQualityDiag.handle_dns_fail(), 0);
}

HWTEST_F(DnsQualityDiagTest, send_dns_report_ShouldReturnZero_WhenCalled_01, TestSize.Level0)
{
    dnsQualityDiag.handler_started = false;
    EXPECT_EQ(dnsQualityDiag.send_dns_report(), 0);
}

HWTEST_F(DnsQualityDiagTest, send_dns_report_ShouldReturnZero_WhenCalled_02, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    EXPECT_EQ(dnsQualityDiag.send_dns_report(), 0);
}

HWTEST_F(DnsQualityDiagTest, send_dns_report_ShouldReturnZero_WhenCalled_03, TestSize.Level0)
{
    std::shared_ptr<NetsysNative::NetDnsResultReport> report;
    report = std::make_shared<NetsysNative::NetDnsResultReport>();
    EXPECT_EQ(dnsQualityDiag.add_dns_report(report), 0);
    EXPECT_TRUE(dnsQualityDiag.report_.size() > 0);

    dnsQualityDiag.handler_started = true;
    EXPECT_EQ(dnsQualityDiag.send_dns_report(), 0);
    EXPECT_EQ(dnsQualityDiag.report_.size(), 0);
}

HWTEST_F(DnsQualityDiagTest, add_dns_report_ShouldReturnZero_WhenCalled_01, TestSize.Level0)
{
    std::shared_ptr<NetsysNative::NetDnsResultReport> report;
    report = std::make_shared<NetsysNative::NetDnsResultReport>();
    EXPECT_EQ(dnsQualityDiag.add_dns_report(report), 0);
    EXPECT_EQ(dnsQualityDiag.report_.size(), 1);
}

HWTEST_F(DnsQualityDiagTest, add_dns_report_ShouldReturnZero_WhenCalled_02, TestSize.Level0)
{
    std::shared_ptr<NetsysNative::NetDnsResultReport> report = nullptr;
    EXPECT_EQ(dnsQualityDiag.add_dns_report(report), 0);
}

HWTEST_F(DnsQualityDiagTest, add_dns_report_ShouldNotAddReport_WhenReportListIsFull, TestSize.Level0)
{
    for (int i = 0; i < MAX_RESULT_SIZE; i++) {
        std::shared_ptr<NetsysNative::NetDnsResultReport> report = std::make_shared<NetsysNative::NetDnsResultReport>();
        dnsQualityDiag.add_dns_report(report);
    }
    std::shared_ptr<NetsysNative::NetDnsResultReport> report = std::make_shared<NetsysNative::NetDnsResultReport>();
    int32_t result = dnsQualityDiag.add_dns_report(report);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(dnsQualityDiag.report_.size(), MAX_RESULT_SIZE);

    uint16_t netId = 1;
    uint16_t uid = 1;
    uint32_t pid = 1;
    int32_t usedtime = 1;
    char name[] = "test";
    uint32_t size = 1;
    int32_t failreason = 1;
    QueryParam queryParam;
    AddrInfo addrinfo;
    EXPECT_EQ(dnsQualityDiag.ReportDnsResult(netId, uid, pid, usedtime, name, size, failreason, queryParam, &addrinfo),
              0);
}

HWTEST_F(DnsQualityDiagTest, load_query_addr_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    const char *defaultAddr = "test";
    EXPECT_EQ(dnsQualityDiag.load_query_addr(defaultAddr), 0);
}

HWTEST_F(DnsQualityDiagTest, HandleEvent_ShouldReturnZero_WhenCalled_01, TestSize.Level0)
{
    dnsQualityDiag.handler_started = false;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get();
    EXPECT_EQ(dnsQualityDiag.HandleEvent(event), 0);
}

HWTEST_F(DnsQualityDiagTest, HandleEvent_ShouldReturnZero_WhenCalled_02, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(DnsQualityEventHandler::MSG_DNS_MONITOR_LOOP);
    EXPECT_EQ(dnsQualityDiag.HandleEvent(event), 0);
}

HWTEST_F(DnsQualityDiagTest, HandleEvent_ShouldReturnZero_WhenCalled_03, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(DnsQualityEventHandler::MSG_DNS_QUERY_FAIL);
    EXPECT_EQ(dnsQualityDiag.HandleEvent(event), 0);
}

HWTEST_F(DnsQualityDiagTest, HandleEvent_ShouldReturnZero_WhenCalled_04, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(DnsQualityEventHandler::MSG_DNS_REPORT_LOOP);
    EXPECT_EQ(dnsQualityDiag.HandleEvent(event), 0);
}

HWTEST_F(DnsQualityDiagTest, HandleEvent_ShouldReturnZero_WhenCalled_05, TestSize.Level0)
{
    dnsQualityDiag.handler_started = true;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(DnsQualityEventHandler::MSG_DNS_NEW_REPORT);
    EXPECT_EQ(dnsQualityDiag.HandleEvent(event), 0);
}

}  // namespace nmd
}  // namespace OHOS
