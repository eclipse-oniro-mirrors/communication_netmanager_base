/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "i_net_detection_callback.h"
#include "inet_addr.h"
#include "net_conn_types.h"
#include "net_link_info.h"
#include "net_monitor.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t INVALID_NET_ID = 0;
constexpr int32_t MIN_NET_ID = 100;
constexpr int32_t MAX_NET_ID = 0xFFFF - 0x400;
using NetDetectionHandler = std::function<void(uint32_t supplierId, bool ifValid)>;
class Network : public virtual RefBase {
public:
    Network(int32_t netId, uint32_t supplierId, NetDetectionHandler handler);
    ~Network();
    bool operator==(const Network &network) const;
    int32_t GetNetId() const;
    bool UpdateBasicNetwork(bool isAvailable_);
    bool UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo);
    NetLinkInfo GetNetLinkInfo() const;
    void UpdateIpAddrs(const NetLinkInfo &netLinkInfo);
    void UpdateInterfaces(const NetLinkInfo &netLinkInfo);
    void UpdateRoutes(const NetLinkInfo &netLinkInfo);
    void UpdateDnses(const NetLinkInfo &netLinkInfo);
    void UpdateMtu(const NetLinkInfo &netLinkInfo);
    void RegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback);
    int32_t UnRegisterNetDetectionCallback(const sptr<INetDetectionCallback> &callback);
    void StartNetDetection();
    uint64_t GetNetWorkMonitorResult();
    void SetDefaultNetWork();
    void ClearDefaultNetWorkNetId();
    void SetExternDetection();

private:
    void StopNetDetection();
    bool CreateBasicNetwork();
    bool ReleaseBasicNetwork();
    void StartDetectionThread();
    void HandleNetMonitorResult(NetDetectionStatus netDetectionState, const std::string &urlRedirect);
    void NotifyNetDetectionResult(NetDetectionResultCode detectionResult, const std::string &urlRedirect);
    NetDetectionResultCode NetDetectionResultConvert(int32_t internalRet);

private:
    int32_t netId_ = 0;
    uint32_t supplierId_ = 0;
    NetLinkInfo netLinkInfo_;
    bool isPhyNetCreated_ = false;
    std::unique_ptr<NetMonitor> netMonitor_ = nullptr;
    NetDetectionHandler  netCallback_;
    NetDetectionStatus netDetectionState_;
    std::string urlRedirect_;
    std::vector<sptr<INetDetectionCallback>> netDetectionRetCallback_;
    bool isExternDetection_ = false;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORK_H