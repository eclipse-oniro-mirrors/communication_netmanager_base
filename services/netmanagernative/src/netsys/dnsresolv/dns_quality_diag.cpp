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

#include <fstream>
#include <sstream>

#include "dns_quality_diag.h"
#include "third_party/musl/include/netdb.h"
#include "net_handle.h"
#include "net_conn_client.h"

namespace OHOS::nmd {
namespace {
using namespace OHOS::NetsysNative;
}

constexpr const char *DNS_DIAG_WORK_THREAD = "DNS_DIAG_WORK_THREAD";
constexpr const char *HW_HICLOUD_ADDR = "http://connectivitycheck.platform.hicloud.com/generate_204";
constexpr const uint32_t MAX_RESULT_SIZE = 32;
constexpr const char* URL_CFG_FILE = "/system/etc/netdetectionurl.conf";
constexpr const char* HTTP_URL_HEADER = "HttpProbeUrl:";
constexpr const char  NEW_LINE_STR = '\n';
constexpr const uint32_t TIME_DELAY = 100;

DnsQualityDiag::DnsQualityDiag() : defaultNetId_(0),
  monitor_loop_delay(TIME_DELAY),
  report_delay(TIME_DELAY),
  handler_started(false),
  handler_(nullptr)
  {
      InitHandler();
      load_query_addr(HW_HICLOUD_ADDR);
  }

DnsQualityDiag &DnsQualityDiag::GetInstance()
{
    static DnsQualityDiag instance;
    return instance;
}

int32_t DnsQualityDiag::InitHandler()
{
    if (handler_ == nullptr) {
        std::shared_ptr<AppExecFwk::EventRunner> runner_ =
            AppExecFwk::EventRunner::Create(DNS_DIAG_WORK_THREAD);
        if (!runner_) {
            NETNATIVE_LOGE("Create net policy work event runner.");
        } else {
            handler_ = std::make_shared<DnsQualityEventHandler>(runner_);
        }
    }
    return 0;
}

int32_t DnsQualityDiag::SendHealthReport(NetsysNative::NetDnsHealthReport healthreport)
{
    for (auto cb: healthListeners_) {
        cb->OnDnsHealthReport(healthreport);
    }

    return 0;
}

int32_t DnsQualityDiag::ReportDnsResult(uint16_t netId, uint16_t uid, uint32_t pid, int32_t usedtime,
    std::string* name, uint32_t size, int32_t failreason, QueryParam queryParam, AddrInfo* addrinfo)
{
    bool reportSizeReachLimit = (report_.size() >= MAX_RESULT_SIZE);

    NETNATIVE_LOGI("ReportDnsResult: %{public}d, %{public}d, %{public}d, %{public}d, %{public}d, %{public}d",
                   netId, uid, pid, usedtime, size, failreason);

    if (!reportSizeReachLimit) {
        NetsysNative::NetDnsResultReport report;
        report.netid_ = netId;
        report.uid_ = uid;
        report.pid_ = pid;
        report.timeused_ = usedtime;
        report.queryresult_ = failreason;
        report.host_ = name;
        int maxSize = 40;
        for (uint8_t i = 0; i < size; i++) {
            NetsysNative::NetDnsResultAddrInfo ai;
            AddrInfo *tmp = &(addrinfo[i]);
            switch (tmp->aiFamily) {
                case AF_INET:
                    ai.type_ = NetsysNative::ADDR_TYPE_IPV4;
                    ai.addr_ = tmp->aiAddr.sa.sa_data;
                    break;
                case AF_INET6:
                    char temp[maxSize] = {'\0'};
                    int ret = sprintf_s(temp, sizeof(temp),
                              "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[0],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[1],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[2],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[3],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[4],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[5],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[6],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[7],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[8],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[9],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[10],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[11],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[12],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[13],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[14],
                              tmp->aiAddr.sin6.sin6_addr.__in6_union.__s6_addr[15]);
                    if (ret == -1) {
                        continue;
                    }
                    ai.type_ = NetsysNative::ADDR_TYPE_IPV6;
                    ai.addr_ = temp;
                    break;
            }
            if (report.addrlist_.size() < MAX_RESULT_SIZE) {
                report.addrlist_.push_back(ai);
            } else {
                break;
            }
            NETNATIVE_LOGI("ReportDnsResult: %{public}s, ", ai.addr_.c_str());
        }
        NETNATIVE_LOGI("ReportDnsResult: %{public}s", report.host_.c_str());
        std::shared_ptr<NetsysNative::NetDnsResultReport> rpt =
            std::make_shared<NetsysNative::NetDnsResultReport>(report);
        auto event = AppExecFwk::InnerEvent::Get(DnsQualityEventHandler::MSG_DNS_NEW_REPORT, rpt);
        handler_->SendEvent(event);
    }

    return 0;
}

int32_t DnsQualityDiag::RegisterResultListener(const sptr<INetDnsResultCallback> &callback, uint32_t timeStep)
{
    report_delay = std::max(report_delay, timeStep);
    resultListeners_.push_back(callback);

    if (handler_started != true) {
        handler_started = true;
        handler_->SendEvent(DnsQualityEventHandler::MSG_DNS_REPORT_LOOP, report_delay);
        handler_->SendEvent(DnsQualityEventHandler::MSG_DNS_MONITOR_LOOP, monitor_loop_delay);
    }
    NETNATIVE_LOGI("RegisterResultListener, %{public}d", report_delay);

    return 0;
}

int32_t DnsQualityDiag::UnregisterResultListener(const sptr<INetDnsResultCallback> &callback)
{
    resultListeners_.remove(callback);
    if (resultListeners_.empty()) {
        handler_started = false;
    }
    NETNATIVE_LOGI("UnregisterResultListener");
    
    return 0;
}

int32_t DnsQualityDiag::RegisterHealthListener(const sptr<INetDnsHealthCallback> &callback)
{
    healthListeners_.push_back(callback);
    handler_started = true;
    handler_->SendEvent(DnsQualityEventHandler::MSG_DNS_MONITOR_LOOP, monitor_loop_delay);
    
    return 0;
}

int32_t DnsQualityDiag::UnregisterHealthListener(const sptr<INetDnsHealthCallback> &callback)
{
    healthListeners_.remove(callback);
    if (healthListeners_.empty()) {
        handler_started = false;
    }
    
    return 0;
}

int32_t DnsQualityDiag::SetLoopDelay(int32_t delay)
{
    monitor_loop_delay = delay;
    return 0;
}

int32_t DnsQualityDiag::query_default_host()
{
    struct addrinfo *res;
    struct queryparam param;
    param.qp_type = 1;

    OHOS::NetManagerStandard::NetHandle netHandle;
    OHOS::NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(netHandle);
    int netid = netHandle.GetNetId();

    NETNATIVE_LOGI("query_default_host: %{public}d, ", netid);

    param.qp_netid = netid;
    getaddrinfo_ext(queryAddr.c_str(), NULL, NULL, &res, &param);
    freeaddrinfo(res);
    return 0;
}

int32_t DnsQualityDiag::handle_dns_loop()
{
    if (handler_started) {
        if (report_.size() == 0) {
            query_default_host();
        }
        handler_->SendEvent(DnsQualityEventHandler::MSG_DNS_MONITOR_LOOP, monitor_loop_delay);
    }
    return 0;
}

int32_t DnsQualityDiag::handle_dns_fail()
{
    if (handler_started) {
        query_default_host();
    }
    return 0;
}

int32_t DnsQualityDiag::send_dns_report()
{
    std::list<NetsysNative::NetDnsResultReport> reportSend(report_);
    report_.clear();
    NETNATIVE_LOGE("send_dns_report (%{public}lu)", reportSend.size());
    for (auto cb: resultListeners_) {
        NETNATIVE_LOGI("send_dns_report cb)");
        cb->OnDnsResultReport(reportSend.size(), reportSend);
    }
    handler_->SendEvent(DnsQualityEventHandler::MSG_DNS_REPORT_LOOP, report_delay);
    return 0;
}

int32_t DnsQualityDiag::add_dns_report(std::shared_ptr<NetsysNative::NetDnsResultReport> report)
{
    NETNATIVE_LOGE("add_dns_report (%{public}s)", report->host_.c_str());
    if (report_.size() < MAX_RESULT_SIZE) {
        report_.push_back(*report);
    }
    return 0;
}

int32_t DnsQualityDiag::load_query_addr(const char* defaultAddr)
{
    if (!std::filesystem::exists(URL_CFG_FILE)) {
        NETNATIVE_LOGE("File not exist (%{public}s)", URL_CFG_FILE);
        queryAddr = defaultAddr;
        return 0;
    }

    std::ifstream file(URL_CFG_FILE);
    if (!file.is_open()) {
        NETNATIVE_LOGE("Open file failed (%{public}s)", strerror(errno));
        queryAddr = defaultAddr;
        return 0;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();
    auto pos = content.find(HTTP_URL_HEADER);
    if (pos != std::string::npos) {
        pos += strlen(HTTP_URL_HEADER);
        queryAddr = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    } else {
        queryAddr = defaultAddr;
    }
    NETNATIVE_LOGI("Get queryAddr url:[%{public}s]]", queryAddr.c_str());

    return 0;
}

int32_t DnsQualityDiag::HandleEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (!handler_started) {
        NETNATIVE_LOGI("DnsQualityDiag Handler not started");
        return 0;
    }
    NETNATIVE_LOGI("DnsQualityDiag Handler event: %{public}d", event->GetInnerEventId());

    switch (event->GetInnerEventId()) {
        case DnsQualityEventHandler::MSG_DNS_MONITOR_LOOP:
            handle_dns_loop();
            break;
        case DnsQualityEventHandler::MSG_DNS_QUERY_FAIL:
            handle_dns_fail();
            break;
        case DnsQualityEventHandler::MSG_DNS_REPORT_LOOP:
            send_dns_report();
            break;
        case DnsQualityEventHandler::MSG_DNS_NEW_REPORT:
            auto report = event->GetSharedObject<NetsysNative::NetDnsResultReport>();
            add_dns_report(report);
            break;
    }
    return 0;
}
} // namespace OHOS::nmd
