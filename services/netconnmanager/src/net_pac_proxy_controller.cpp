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

#include "net_pac_proxy_controller.h"

#include <shared_mutex>

#include "net_conn_service.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *LOCAL_PROXY_HOST = "127.0.0.1";
constexpr int32_t LOCAL_PORT_START = 1024;
constexpr int32_t LOCAL_PORT_END = 65535;
} // namespace

NetPacProxyController::NetPacProxyController(NetConnService &service) : service_(service) {}

bool NetPacProxyController::PacDecision::operator==(const PacDecision &other) const
{
    return source == other.source && netId == other.netId && url == other.url;
}

void NetPacProxyController::Refresh()
{
    std::call_once(queueOnce_, [this]() { queue_ = std::make_shared<ffrt::queue>("NetConnWifiPac"); });
    std::weak_ptr<NetConnService> wp = service_.weak_from_this();
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (taskHandle_ != nullptr) {
        queue_->cancel(taskHandle_);
    }
    taskHandle_ = queue_->submit_h(
        [wp, this]() {
            if (auto service = wp.lock()) {
                ApplyWifiPac();
            }
        },
        ffrt::task_attr().name("ApplyWifiPac"));
}

NetPacProxyController::WifiPacPause::WifiPacPause(NetPacProxyController &controller)
    : controller_(controller), lock_(controller.operationMutex_)
{
    if (controller_.StopServer()) {
        controller_.SendEffectiveProxyBroadcast();
    }
}

NetPacProxyController::WifiPacPause::~WifiPacPause()
{
    lock_.unlock();
    controller_.Refresh();
}

NetPacProxyController::WifiPacPause NetPacProxyController::PauseWifiPac()
{
    return WifiPacPause(*this);
}

NetPacProxyController::PacDecision NetPacProxyController::Decide()
{
    PacDecision decision;
    if (IsGlobalPacRunning()) {
        decision.source = PacSource::GLOBAL;
        return decision;
    }
    HttpProxy globalProxy;
    service_.LoadGlobalHttpProxy(NetConnService::ACTIVE, globalProxy);
    if (!globalProxy.GetHost().empty()) {
        decision.source = PacSource::GLOBAL;
        return decision;
    }
    if (globalProxy.GetUserId() < 0) {
        return decision;
    }
    std::shared_lock<ffrt::shared_mutex> lock(service_.defaultNetSupplierMutex_);
    auto supplier = service_.defaultNetSupplier_;
    if (supplier == nullptr || supplier->GetNetSupplierType() != BEARER_WIFI) {
        return decision;
    }
    auto network = supplier->GetNetwork();
    if (network == nullptr || !network->IsConnected()) {
        return decision;
    }
    HttpProxy proxy = network->GetHttpProxy();
    if (proxy.GetHost().empty() && !proxy.GetPacUrl().empty()) {
        decision.source = PacSource::WIFI;
        decision.netId = supplier->GetNetId();
        decision.url = proxy.GetPacUrl();
    }
    return decision;
}

bool NetPacProxyController::HasWifiPac()
{
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (runtime_.port > 0) {
            return true;
        }
    }
    std::shared_lock<ffrt::shared_mutex> lock(service_.defaultNetSupplierMutex_);
    auto supplier = service_.defaultNetSupplier_;
    HttpProxy proxy;
    if (supplier != nullptr && supplier->GetNetSupplierType() == BEARER_WIFI) {
        supplier->GetHttpProxy(proxy);
    }
    return !proxy.GetPacUrl().empty();
}

bool NetPacProxyController::IsGlobalPacRunning()
{
    std::lock_guard<std::mutex> lock(service_.netPacProxyServerMutex_);
    return service_.netPACProxyServer_ != nullptr && service_.netPACProxyServer_->IsRunning();
}

void NetPacProxyController::ApplyWifiPac()
{
    std::unique_lock<ffrt::mutex> lock(operationMutex_);
    PacDecision decision = Decide();
    if (decision.source != PacSource::WIFI) {
        if (StopServer()) {
            SendEffectiveProxyBroadcast();
        }
        return;
    }
    Runtime runtime;
    {
        std::lock_guard<std::mutex> stateLock(stateMutex_);
        runtime = runtime_;
    }
    if (runtime.port > 0 && runtime.decision == decision && server_ && server_->IsRunning()) {
        SendEffectiveProxyBroadcast();
        return;
    }
    bool stopped = StopServer();
    lock.unlock();
    std::string script;
    bool downloaded = NetPACManager::DownloadPACScript(decision.url, script);
    lock.lock();
    if (!(Decide() == decision)) {
        if (stopped) {
            SendEffectiveProxyBroadcast();
        }
        return;
    }
    if (!downloaded || !service_.GetNetPacManager()->LoadPACScript(script) || !StartServer(decision)) {
        NETMGR_LOG_E("Load wifi PAC or start local proxy failed");
    }
    SendEffectiveProxyBroadcast();
}

bool NetPacProxyController::StartServer(const PacDecision &decision)
{
    int32_t port = ProxyServer::FindAvailablePort(LOCAL_PORT_START, LOCAL_PORT_END);
    if (port <= 0) {
        return false;
    }
    auto server = std::make_shared<ProxyServer>(port, 0);
    server->SetFindPacProxyFunction([this](auto url, auto host) {
        std::string proxy;
        service_.GetNetPacManager()->FindProxyForURL(url, host, proxy);
        return proxy;
    });
    if (!server->Start()) {
        return false;
    }
    server_ = server;
    std::lock_guard<std::mutex> lock(stateMutex_);
    runtime_ = {decision, port};
    return true;
}

bool NetPacProxyController::StopServer()
{
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        runtime_ = {};
    }
    if (!server_) {
        return false;
    }
    server_->Stop();
    server_.reset();
    return true;
}

void NetPacProxyController::FillLocalProxy(int32_t netId, HttpProxy &httpProxy)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (runtime_.port <= 0 || runtime_.decision.netId != netId || runtime_.decision.url != httpProxy.GetPacUrl() ||
        !httpProxy.GetHost().empty()) {
        return;
    }
    httpProxy.SetHost(std::string(LOCAL_PROXY_HOST));
    httpProxy.SetPort(static_cast<uint16_t>(runtime_.port));
}

void NetPacProxyController::SendEffectiveProxyBroadcast()
{
    HttpProxy proxy;
    service_.GetDefaultHttpProxy(INVALID_NET_ID, proxy);
    service_.SendHttpProxyChangeBroadcast(proxy);
}
} // namespace NetManagerStandard
} // namespace OHOS
