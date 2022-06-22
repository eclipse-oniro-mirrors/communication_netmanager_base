/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include <future>
#include <list>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstring>
#include <securec.h>
#include "net_mgr_log_wrapper.h"
#include "netsys_controller.h"
#include "net_monitor.h"

namespace OHOS {
namespace NetManagerStandard {
static const std::string DEFAULT_PORTAL_HTTP_URL = "http://connectivitycheck.platform.hicloud.com/generate_204";
static const std::string DEFAULT_PORTAL_HTTPS_URL = "https://connectivitycheck.platform.hicloud.com/generate_204";
static constexpr int32_t INIT_REEVALUATE_DELAY_MS = 8*1000;
static constexpr int32_t MAX_FAILED_REEVALUATE_DELAY_MS = 10 * 60 * 1000;
static constexpr int32_t SUCCESSED_REEVALUATE_DELAY_MS = 30 * 1000;
static constexpr int32_t CAPTIVE_PORTAL_REEVALUATE_DELAY_MS = 10 * 60 * 1000;
static constexpr int32_t DOUBLE = 2; // so ugly

NetMonitor::NetMonitor(uint32_t netId, SocketFactory& sockFactory, NetConnAsync &async)
    :netId_(netId), sockFactory_(sockFactory), async_(async)
{
}

NetMonitor::~NetMonitor()
{
    Stop();
}

void NetMonitor::Start()
{
    NETMGR_LOG_I("NetMonitor[%{public}d] start evaluation", netId_);
    if (IsEvaluating()) {
        return ;
    }

    evaluating_ = true;
    reevaluateDelay_ = INIT_REEVALUATE_DELAY_MS;
    evaluationThread_ = std::thread([&]() {
        while (evaluating_) {
            OnEvaluating();
        }
    });
}

void NetMonitor::Stop()
{
    NETMGR_LOG_I("NetMonitor[%{public}d] stop evaluation", netId_);
    evaluating_ = false;
    evaluationTimerCond_.notify_all();
    if (evaluationThread_.joinable()) {
        evaluationThread_.join();
    }
}

void NetMonitor::Restart()
{
    NETMGR_LOG_I("NetMonitor[%{public}d] restart evaluation", netId_);
    Stop();
    Start();
}

bool NetMonitor::IsEvaluating() const
{
    return evaluating_;
}

bool NetMonitor::IsValidated() const
{
    return result_.IsSuccessful();
}

HttpProbeResult NetMonitor::GetEvaluationResult() const
{
    return result_;
}

void NetMonitor::OnEvaluating()
{
    HttpProbeResult result;

    result = SendParallelHttpProbes(DEFAULT_PORTAL_HTTP_URL, DEFAULT_PORTAL_HTTPS_URL);
    
    std::unique_lock<std::mutex> lock(evaluationTimerMtx_);
    
    if (result.IsPortal()) {
        reevaluateDelay_ = CAPTIVE_PORTAL_REEVALUATE_DELAY_MS;
    } else if (result.IsSuccessful()) {
        reevaluateDelay_ = SUCCESSED_REEVALUATE_DELAY_MS;
    } else {
        NETMGR_LOG_I("NetMonitor[%{public}d] evaluation failed, code[%{public}d]", netId_, result.GetCode());
        reevaluateDelay_ *= DOUBLE;
        if (reevaluateDelay_ >= MAX_FAILED_REEVALUATE_DELAY_MS) {
            reevaluateDelay_ = MAX_FAILED_REEVALUATE_DELAY_MS;
        }
    }

    if (result != result_) {
        result_ = result;
        OnProbeResultChanged();
    }

    if (evaluating_) {
        evaluationTimerCond_.wait_for(lock, std::chrono::milliseconds(reevaluateDelay_));
    }
}

void NetMonitor::OnProbeResultChanged()
{
    NetDetectionResultCode code;

    if (result_.IsPortal()) {
        code = NET_DETECTION_CAPTIVE_PORTAL;
    } else if (result_.IsSuccessful()) {
        code = NET_DETECTION_SUCCESS;
    } else {
        code = NET_DETECTION_FAIL;
    }

    async_.CallbackOnNetDetectionResultChanged(netId_, code, result_.GetRedirectUrl());
}

HttpProbeResult NetMonitor::SendParallelHttpProbes(const Url& httpUrl, const Url& httpsUrl)
{
    auto now = std::chrono::system_clock::now();
    auto httpResult = std::async(std::launch::async,
        std::bind(&NetMonitor::SendDnsAndHttpProbes, this, httpUrl.ToString(), HttpProbe::PROBE_HTTP));
    auto httpsResult = std::async(std::launch::async,
        std::bind(&NetMonitor::SendDnsAndHttpProbes, this, httpsUrl.ToString(), HttpProbe::PROBE_HTTPS));

    httpResult.wait();
    httpsResult.wait();
    NETMGR_LOG_I("NetMonitor[%{public}d] send http&https probes cost %{public}lld ms", netId_,
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());

    if (httpResult.get().IsPortal()) {
        return httpResult.get();
    }
    return httpsResult.get();
}

HttpProbeResult NetMonitor::SendDnsAndHttpProbes(const Url& url, HttpProbe::ProbeType probeType)
{
    SendDnsProbe(url.GetHost());
    return SendHttpProbe(url.ToString(), probeType);
}

HttpProbeResult NetMonitor::SendHttpProbe(const std::string& url, HttpProbe::ProbeType probeType)
{
    int sockFd = sockFactory_.CreateSocket(AF_INET, SOCK_STREAM, 0);
    HttpProbeResult result;
    if (sockFd > 0) {
        HttpProbe httpProbe(probeType, url, sockFd);
        if (!httpProbe.HasError()) {
            result = httpProbe.GetResult();
        } else {
            NETMGR_LOG_W("NetMonitor[%{public}d] Http probe failed,[ %{public}s]", netId_,
                httpProbe.ErrorString().c_str());
        }
        sockFactory_.DestroySocket(sockFd);
    } else {
        NETMGR_LOG_W("Create socket failed");
    }

    return result;
}

void NetMonitor::SendDnsProbe(const std::string& host)
{
    auto now = std::chrono::system_clock::now();
    std::list<std::string> addrList;
    struct addrinfo hints;
    struct addrinfo *result = nullptr;

    (void)memset_s(&hints, sizeof(hints), 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;

    int32_t err = 0;
    // call getaddrinfo to resolve host
    
    for (struct addrinfo *addrInfo = result; addrInfo != nullptr; addrInfo = addrInfo->ai_next) {
        struct in_addr addr;
        struct sockaddr_in* addrin = reinterpret_cast<struct sockaddr_in*>(addrInfo->ai_addr);
        addr.s_addr = addrin->sin_addr.s_addr;
        addrList.push_back(inet_ntoa(addr));
    }

    if (result) {
        freeaddrinfo(result);
    }

    NETMGR_LOG_I("NetMonitor[%{public}d] send dns probe cost %{public}lld ms", netId_,
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());
    if (addrList.size() > 0) {
        // dns probe success...
    } else {
        NETMGR_LOG_I("Resolve url failed %{public}d", err);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
