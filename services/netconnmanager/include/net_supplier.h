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

#ifndef NET_CONN_NET_SUPPLIER_H
#define NET_CONN_NET_SUPPLIER_H

#include <string>
#include <set>
#include <vector>
#include <map>
#include <future>
#include "network.h"
#include "net_request.h"
#include "net_monitor.h"
#include "net_supplier_info.h"
#include "net_conn_async.h"
#include "i_net_supplier_callback.h"
#include "i_net_detection_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class NetCaps {
public:
    NetCaps() = default;

    NetCaps(const std::set<NetCap> &caps)
    {
        for (auto cap : caps) {
            InsertNetCap(cap);
        }
    }

    ~NetCaps() = default;

    static bool IsValidNetCap(NetCap cap)
    {
        return (cap >= 0) || (cap < NET_CAPABILITY_INTERNAL_DEFAULT);
    }

    void InsertNetCap(NetCap cap)
    {
        if (IsValidNetCap(cap)) {
            caps_ |= (1<<cap);
        }
    }

    void RemoveNetCap(NetCap cap)
    {
        if (IsValidNetCap(cap)) {
            caps_ &= ~(1<<cap);
        }
    }

    bool HasNetCap(NetCap cap) const
    {
        return (caps_ >> cap) & 1;
    }

    std::set<NetCap> ToSet() const
    {
        std::set<NetCap> ret;
        for (auto cap = static_cast<NetCap>(0);
            cap < NET_CAPABILITY_INTERNAL_DEFAULT; cap = static_cast<NetCap>(cap+1)) {
            if (HasNetCap(cap)) {
                ret.insert(cap);
            }
        }
        return ret;
    }

private:
    uint32_t caps_ {0};
};

class NetSupplier : public virtual RefBase {
public:
    NetSupplier(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &caps, NetConnAsync &async);
    
    virtual ~NetSupplier();

    uint32_t GetId() const;
   
    uint32_t GetNetId() const;

    NetBearType GetBearerType() const;

    std::string GetIdent() const;

    std::set<NetCap> GetCaps() const;

    sptr<Network> GetNetwork() const;

    sptr<NetMonitor> GetNetMonitor() const;

    sptr<NetHandle> GetNetHandle() const;

    sptr<NetSupplierInfo> GetSupplierInfo() const;

    sptr<NetLinkInfo> GetNetLinkInfo() const;

    sptr<NetAllCapabilities> GetNetAllCapabilities() const;
    
    int32_t GetCurrentScore() const;
    
    bool IsAvailable() const;

    bool IsRequested() const;

    bool HasNetCaps(const std::set<NetCap>& caps) const;

    bool HasNetCap(NetCap cap) const;

    void InsertNetCap(NetCap cap);

    void RemoveNetCap(NetCap cap);

    void UpdateNetSupplierInfo(sptr<NetSupplierInfo> supplierInfo);

    void UpdateNetLinkInfo(sptr<NetLinkInfo> linkInfo);

    void SetSupplierCallback(sptr<INetSupplierCallback> supplierCb);

    void RegisterNetDetectionCallback(sptr<INetDetectionCallback> callback);

    void UnregisterNetDetectionCallback(sptr<INetDetectionCallback> callback);

    bool SatisfiyNetRequest(sptr<NetRequest> netRequest);
    
    void AddNetRequest(sptr<NetRequest> netRequest);

    void RemoveNetRequest(sptr<NetRequest> netRequest);

    void RemoveAllNetRequests();
    
    void NotifyNetDetectionResult(NetDetectionResultCode detectionResult, const std::string &urlRedirect);

private:
    void RequestNetwork();

    void ReleaseNetwork();
    
    void SetNetConnState(NetConnState netConnState);

    void NotifyNetRequestCallbacks(int32_t cmd);

private:
    uint32_t id_;
    NetBearType bearerType_;
    std::string ident_;
    NetCaps caps_;
    NetConnAsync& async_;
    sptr<NetAllCapabilities> allCaps_;
    sptr<NetSupplierInfo> supplierInfo_;
    sptr<NetLinkInfo> linkInfo_;
    sptr<Network> network_;
    sptr<NetHandle> netHandle_;
    sptr<NetMonitor> netMonitor_;
    sptr<INetSupplierCallback> netSupplierCb_;
    std::list<sptr<INetDetectionCallback>> netDetectionCbs_;
    std::set<sptr<NetRequest>> netReqs_;
    NetConnState netConnState_ {NET_CONN_STATE_UNKNOWN};
    std::future<void> reqRelAsync_;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // NET_CONN_NET_SUPPLIER_H
