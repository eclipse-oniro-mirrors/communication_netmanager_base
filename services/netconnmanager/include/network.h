/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NETWORK_H
#define NETWORK_H

#include "event_report.h"
#include "i_net_detection_callback.h"
#include "i_net_monitor_callback.h"
#include "inet_addr.h"
#include "net_conn_event_handler.h"
#include "net_conn_types.h"
#include "net_link_info.h"
#include "net_monitor.h"
#include "net_supplier_info.h"
#include "route.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
using NetDetectionHandler = std::function<void(uint32_t supplierId, NetDetectionStatus netState)>;
class Network : public virtual RefBase, public INetMonitorCallback, public std::enable_shared_from_this<Network> {
public:
    Network(int32_t netId, uint32_t supplierId, const NetDetectionHandler &handler, NetBearType bearerType,
            const std::shared_ptr<NetConnEventHandler> &eventHandler);
    ~Network() = default;
    bool operator==(const Network &network) const;
    int32_t GetNetId() const;
    uint32_t GetSupplierId() const;
    bool UpdateBasicNetwork(bool isAvailable_);
    bool UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo);
    NetLinkInfo GetNetLinkInfo() const;
    void UpdateIpAddrs(const NetLinkInfo &newNetLinkInfo);
    void HandleUpdateIpAddrs(const NetLinkInfo &newNetLinkInfo);
    void UpdateInterfaces(const NetLinkInfo &newNetLinkInfo);
    void UpdateRoutes(const NetLinkInfo &newNetLinkInfo);
    void UpdateDns(const NetLinkInfo &netLinkInfo);
    void UpdateMtu(const NetLinkInfo &netLinkInfo);
    void UpdateTcpBufferSize(const NetLinkInfo &netLinkInfo);
    void UpdateStatsCached(const NetLinkInfo &netLinkInfo);
    void RegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback);
    int32_t UnRegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback);
    void StartNetDetection(bool needReport);
    void SetNetCaps(const std::set<NetCap> &netCaps);
    void SetDefaultNetWork();
    void ClearDefaultNetWorkNetId();
    bool IsConnecting() const;
    bool IsConnected() const;
    void UpdateNetConnState(NetConnState netConnState);
    void UpdateGlobalHttpProxy(const HttpProxy &httpProxy);
    void NetDetectionForDnsHealth(bool dnsHealthSuccess);

    void OnHandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect) override;

    bool ResumeNetworkInfo();

private:
    void StopNetDetection();
    bool CreateBasicNetwork();
    bool CreateVirtualNetwork();
    bool ReleaseBasicNetwork();
    bool ReleaseVirtualNetwork();
    void InitNetMonitor();
    void HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect);
    void NotifyNetDetectionResult(NetDetectionResultCode detectionResult, const std::string &urlRedirect);
    NetDetectionResultCode NetDetectionResultConvert(int32_t internalRet);
    void SendConnectionChangedBroadcast(const NetConnState &netConnState) const;
    void SendSupplierFaultHiSysEvent(NetConnSupplerFault errorType, const std::string &errMsg);
    void ResetNetlinkInfo();
    bool IsDetectionForDnsSuccess(NetDetectionStatus netDetectionState, bool dnsHealthSuccess);
    bool IsDetectionForDnsFail(NetDetectionStatus netDetectionState, bool dnsHealthSuccess);
    bool IsAddrInOtherNetwork(const INetAddr &netAddr);
    bool IsIfaceNameInUse();

private:
    int32_t netId_ = 0;
    uint32_t supplierId_ = 0;
    NetLinkInfo netLinkInfo_;
    NetConnState state_ = NET_CONN_STATE_UNKNOWN;
    NetDetectionStatus detectResult_ = UNKNOWN_STATE;
    std::atomic_bool isPhyNetCreated_ = false;
    std::atomic_bool isVirtualCreated_ = false;
    std::shared_ptr<NetMonitor> netMonitor_ = nullptr;
    NetDetectionHandler netCallback_;
    NetBearType netSupplierType_;
    std::vector<sptr<INetDetectionCallback>> netDetectionRetCallback_;
    std::shared_ptr<NetConnEventHandler> eventHandler_;
    std::atomic<bool> isDetectingForDns_ = false;
    std::set<NetCap> netCaps_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORK_H
