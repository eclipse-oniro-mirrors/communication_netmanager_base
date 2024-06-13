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

#ifndef NETSYS_DNS_QUALITY_DIAG_H
#define NETSYS_DNS_QUALITY_DIAG_H

#include <iostream>
#include <map>

#include "dns_resolv_config.h"
#include "netnative_log_wrapper.h"
#include "dns_quality_event_handler.h"
#include "i_net_dns_health_callback.h"
#include "i_net_dns_result_callback.h"
#include "netsys_net_dns_result_data.h"
#include "dns_config_client.h"

#if DNS_CONFIG_DEBUG
#ifdef DNS_CONFIG_PRINT
#undef DNS_CONFIG_PRINT
#endif
#define DNS_CONFIG_PRINT(fmt, ...) NETNATIVE_LOGI("DNS" fmt, ##__VA_ARGS__)
#else
#define DNS_CONFIG_PRINT(fmt, ...)
#endif

namespace OHOS::nmd {
class DnsQualityDiag {
public:
    ~DnsQualityDiag() = default;

    static DnsQualityDiag &GetInstance();

    // for net_conn_service
    int32_t ReportDnsResult(uint16_t netId, uint16_t uid, uint32_t pid, int32_t usedtime, char* name,
                            uint32_t size, int32_t failreason, QueryParam param, AddrInfo* addrinfo);

    int32_t RegisterResultListener(const sptr<NetsysNative::INetDnsResultCallback> &callback, uint32_t timeStep);

    int32_t UnregisterResultListener(const sptr<NetsysNative::INetDnsResultCallback> &callback);

    int32_t RegisterHealthListener(const sptr<NetsysNative::INetDnsHealthCallback> &callback);

    int32_t UnregisterHealthListener(const sptr<NetsysNative::INetDnsHealthCallback> &callback);

    int32_t SetLoopDelay(int32_t delay);

    int32_t HandleEvent(const AppExecFwk::InnerEvent::Pointer &event);

private:
    DnsQualityDiag();

    std::mutex cacheMutex_;

    std::mutex resultListenersMutex_;

    std::atomic_uint defaultNetId_;

    uint32_t monitor_loop_delay;

    uint32_t report_delay;

    std::atomic_bool handler_started;

    std::string queryAddr;

    std::list<sptr<NetsysNative::INetDnsResultCallback>> resultListeners_;

    std::list<sptr<NetsysNative::INetDnsHealthCallback>> healthListeners_;

    std::shared_ptr<DnsQualityEventHandler> handler_;

    std::list<NetsysNative::NetDnsResultReport> report_;

    int32_t SendHealthReport(NetsysNative::NetDnsHealthReport healthreport);
    int32_t InitHandler();
    int32_t query_default_host();
    int32_t handle_dns_loop();
    int32_t handle_dns_fail();
    int32_t send_dns_report();
    int32_t add_dns_report(std::shared_ptr<NetsysNative::NetDnsResultReport> report);
    int32_t load_query_addr(const char* defaultAddr);
    int32_t ParseReportAddr(uint32_t size, AddrInfo* addrinfo, NetsysNative::NetDnsResultReport &report);
};
} // namespace OHOS::nmd
#endif // NETSYS_DNS_QUALITY_DIAG_H
