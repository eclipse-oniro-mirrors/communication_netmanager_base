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
    /**
     * Construct a new NetCaps
     *
     */
    NetCaps() = default;

    /**
     * Construct a new NetCaps and insert all from caps
     *
     * @param caps caps to insert
     */
    NetCaps(const std::set<NetCap> &caps)
    {
        for (auto cap : caps) {
            InsertNetCap(cap);
        }
    }

    /**
     * Destroy the NetCaps
     *
     */
    ~NetCaps() = default;

    /**
     * Determine if a cap is valid or not
     *
     * @param cap cap to check
     * @return bool cap is valid or not
     */
    static bool IsValidNetCap(NetCap cap)
    {
        return (cap >= 0) || (cap < NET_CAPABILITY_INTERNAL_DEFAULT);
    }

    /**
     * Insert a cap
     *
     * @param cap cap to insert
     */
    void InsertNetCap(NetCap cap)
    {
        if (IsValidNetCap(cap)) {
            caps_ |= (1 << cap);
        }
    }

    /**
     * Remove if cap exist
     *
     * @param cap cap to remove
     */
    void RemoveNetCap(NetCap cap)
    {
        if (IsValidNetCap(cap)) {
            caps_ &= ~(1 << cap);
        }
    }

    /**
     * Determine cap exist or not
     *
     * @param cap cap to check
     * @return bool cap exist or not
     */
    bool HasNetCap(NetCap cap) const
    {
        return (caps_ >> cap) & 1;
    }

    /**
     * Restorage all caps to a std::set<NetCap>
     *
     * @return std::set<NetCap> With all caps
     */
    std::set<NetCap> ToSet() const
    {
        std::set<NetCap> ret;
        for (auto cap = static_cast<NetCap>(0); cap < NET_CAPABILITY_INTERNAL_DEFAULT;
             cap = static_cast<NetCap>(cap + 1)) {
            if (HasNetCap(cap)) {
                ret.insert(cap);
            }
        }
        return ret;
    }

private:
    uint32_t caps_{0};
};

class NetSupplier : public virtual RefBase {
public:
    /**
     * Construct a new NetSupplier
     *
     * @param bearerType Network bearerType
     * @param ident Network ident
     * @param caps  Network caps
     * @param async Async callback
     */
    NetSupplier(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &caps, NetConnAsync &async);

    /**
     * Destroy the NetSupplier
     *
     */
    virtual ~NetSupplier();

    /**
     * Get supplier id
     *
     * @return uint32_t supplier id
     */
    uint32_t GetId() const;

    /**
     * Get the network id of this supplier
     *
     * @return uint32_t network id
     */
    uint32_t GetNetId() const;

    /**
     * Get the bearer type
     *
     * @return NetBearType bearer type
     */
    NetBearType GetBearerType() const;

    /**
     * Get the ident
     *
     * @return std::string ident
     */
    std::string GetIdent() const;

    /**
     * Get the caps in std::set<NetCap>
     *
     * @return std::set<NetCap> caps
     */
    std::set<NetCap> GetCaps() const;

    /**
     * Get the network
     *
     * @return sptr<Network> network
     */
    sptr<Network> GetNetwork() const;

    /**
     * Get the net monitor
     *
     * @return sptr<NetMonitor> net monitor
     */
    sptr<NetMonitor> GetNetMonitor() const;

    /**
     * Get the net handle
     *
     * @return sptr<NetHandle> net handle
     */
    sptr<NetHandle> GetNetHandle() const;

    /**
     * Get the supplier's info
     *
     * @return sptr<NetSupplierInfo> supplier's info
     */
    sptr<NetSupplierInfo> GetSupplierInfo() const;

    /**
     * Get the net link info
     *
     * @return sptr<NetLinkInfo> net link info
     */
    sptr<NetLinkInfo> GetNetLinkInfo() const;

    /**
     * Get the net all capabilities
     *
     * @return sptr<NetAllCapabilities>  all capabilities
     */
    sptr<NetAllCapabilities> GetNetAllCapabilities() const;

    /**
     * Get the current score
     *
     * @return int32_t current score, 0 is min, 100 is max
     */
    int32_t GetCurrentScore() const;

    /**
     * Determine network is available or not
     *
     * @return bool Network is available or not
     */
    bool IsAvailable() const;

    /**
     * Determine network is requested or not
     *
     * @return Network is requested or not
     */
    bool IsRequested() const;

    /**
     * Determine supplier has all of caps
     *
     * @param caps caps to check
     * @return bool Has caps or not
     */
    bool HasNetCaps(const std::set<NetCap> &caps) const;

    /**
     * Determine supplier has cap
     *
     * @param cap cap to check
     * @return Has cap or not
     */
    bool HasNetCap(NetCap cap) const;

    /**
     * Insert a cap
     *
     * @param cap cap to insert
     */
    void InsertNetCap(NetCap cap);

    /**
     * Remove a cap
     *
     * @param cap cap to remove
     */
    void RemoveNetCap(NetCap cap);

    /**
     * Update net supplier info
     *
     * @param supplierInfo Supplier info use to update
     */
    void UpdateNetSupplierInfo(sptr<NetSupplierInfo> supplierInfo);

    /**
     * Update net link info
     *
     * @param linkInfo Net link info use to update
     */
    void UpdateNetLinkInfo(sptr<NetLinkInfo> linkInfo);

    /**
     * Set the supplier callback
     *
     * @param supplierCb supplier callback
     */
    void SetSupplierCallback(sptr<INetSupplierCallback> supplierCb);

    /**
     * Register a net detection callback
     *
     * @param callback net detection callback use to register
     */
    void RegisterNetDetectionCallback(sptr<INetDetectionCallback> callback);

    /**
     * Unregister a net detection callback
     *
     * @param callback net detection callback use to unregister
     */
    void UnregisterNetDetectionCallback(sptr<INetDetectionCallback> callback);

    /**
     * Determine a request is satisfied to this supplier
     *
     * @param netRequest Net request to satisfy
     * @return bool Net request is satisfied or not
     */
    bool SatisfiyNetRequest(sptr<NetRequest> netRequest);

    /**
     * Add a satisfied net request
     *
     * @param netRequest Net request to add
     */
    void AddNetRequest(sptr<NetRequest> netRequest);

    /**
     * Remove a satisfied net request if exist
     *
     * @param netRequest Net request to remove
     */
    void RemoveNetRequest(sptr<NetRequest> netRequest);

    /**
     * Remove all satisfied net requests
     *
     */
    void RemoveAllNetRequests();

    /**
     * Notify to all of registered net detection callbacks that detection result was changed
     *
     * @param detectionResult Detection result status
     * @param urlRedirect Detection result redirect url
     */
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
    NetConnAsync &async_;
    sptr<NetAllCapabilities> allCaps_;
    sptr<NetSupplierInfo> supplierInfo_;
    sptr<NetLinkInfo> linkInfo_;
    sptr<Network> network_;
    sptr<NetHandle> netHandle_;
    sptr<NetMonitor> netMonitor_;
    sptr<INetSupplierCallback> netSupplierCb_;
    std::list<sptr<INetDetectionCallback>> netDetectionCbs_;
    std::set<sptr<NetRequest>> netReqs_;
    NetConnState netConnState_{NET_CONN_STATE_UNKNOWN};
    std::future<void> reqRelAsync_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_NET_SUPPLIER_H
