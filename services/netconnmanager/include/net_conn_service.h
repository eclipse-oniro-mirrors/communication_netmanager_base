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

#ifndef NET_CONN_SERVICE_H
#define NET_CONN_SERVICE_H

#include "singleton.h"
#include "system_ability.h"
#include "net_conn_service_stub.h"
#include "scheduler.h"
#include "net_supplier.h"
#include "net_request.h"
#include "net_conn_async.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnService : public std::enable_shared_from_this<NetConnService>,
                       public SystemAbility,
                       public NetConnServiceStub,
                       public NetConnAsync {
    DECLARE_DELAYED_SINGLETON(NetConnService)
    DECLARE_SYSTEM_ABILITY(NetConnService)
public:
    /**
     * The interface is register the network
     *
     * @param bearerType Bearer Network Type
     * @param ident Unique identification of mobile phone card
     * @param netCaps Network capabilities registered by the network supplier
     * @param supplierId out param, return supplier id
     *
     * @return function result
     */
    int32_t RegisterNetSupplier(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps,
                                uint32_t &supplierId) override;

    /**
     * The interface is unregister the network
     *
     * @param supplierId The id of the network supplier
     *
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t UnregisterNetSupplier(uint32_t supplierId) override;

    /**
     * Register supplier callback
     *
     * @param supplierId The id of the network supplier
     * @param callback INetSupplierCallback callback interface
     *
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback) override;

    /**
     * Register net connection callback
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback) override;

    /**
     * Register net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     * @param timeoutMS net connection time out
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                    const uint32_t &timeoutMS) override;

    /**
     * Unregister net connection callback
     *
     * @return Returns 0, successfully unregister net connection callback, otherwise it will fail
     */
    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback) override;

    int32_t UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState) override;
    /**
     * The interface is update network connection status information
     *
     * @param supplierId The id of the network supplier
     * @param netSupplierInfo network connection status information
     *
     * @return Returns 0, successfully update the network connection status information, otherwise it will fail
     */
    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) override;

    /**
     * The interface is update network link attribute information
     *
     * @param supplierId The id of the network supplier
     * @param netLinkInfo network link attribute information
     *
     * @return Returns 0, successfully update the network link attribute information, otherwise it will fail
     */
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) override;

    /**
     * The interface names which NetBearType is equal than bearerType
     *
     * @param bearerType Network bearer type
     * @param ifaceNames save the obtained ifaceNames
     * @return Returns 0, successfully get the network link attribute iface name, otherwise it will fail
     */
    int32_t GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames) override;

    /**
     * The interface is get the iface name for network
     *
     * @param bearerType Network bearer type
     * @param ident Unique identification of mobile phone card
     * @param ifaceName save the obtained ifaceName
     * @return Returns 0, successfully get the network link attribute iface name, otherwise it will fail
     */
    int32_t GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName) override;

    /**
     * Register network detection return result method
     *
     * @param netId  Network ID
     * @param callback The callback of INetDetectionCallback interface
     * @return int32_t  Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;

    /**
     * Unregister network detection return result method
     *
     * @param netId Network ID
     * @param callback  The callback of INetDetectionCallback interface
     * @return int32_t  Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;

    /**
     * The interface of network detection called by the application
     *
     * @param netId network ID
     * @return int32_t Whether the network probe is successful
     */
    int32_t NetDetection(int32_t netId) override;

    /**
     * Get current default net id
     *
     * @param netId Current default net id alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetDefaultNet(int32_t &netId) override;

    /**
     * Determine if NetConnService has a default net
     *
     * @param flag Boolean flag alias if has default net or not
     * @return int32_t 0 for success, others are failed
     */
    int32_t HasDefaultNet(bool &flag) override;

    /**
     * Resolve host addresses use the network represented by the netId
     *
     * @param host Host name to resolve
     * @param netId The network's id which use to resolve
     * @param addrList Resolved addresses list alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList) override;

    /**
     * Same as GetAddressesByName, but only return one resolved address
     *
     * @param host Host name to resolve
     * @param netId The network's id which use to resolv
     * @param addr Resolved address alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr) override;

    /**
     * Get the specific network list those matched bearerType
     *
     * @param bearerType NetBearType to match
     * @param netIdList matched networ list alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetSpecificNet(NetBearType bearerType, std::list<int32_t> &netIdList) override;

    /**
     * Get all registered networks
     *
     * @param netIdList registered networks list alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetAllNets(std::list<int32_t> &netIdList) override;

    /**
     * Get the network's id match spedific uid
     *
     * @param uid Network's uid to match
     * @param netId Match network's id alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetSpecificUidNet(int32_t uid, int32_t &netId) override;

    /**
     * Get network's connection properties which network's id is netId
     *
     * @param netId network's id
     * @param info  network's connection properties alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetConnectionProperties(int32_t netId, NetLinkInfo &info) override;

    /**
     * Get networks all capabilities which network's id is netId
     *
     * @param netId network's id
     * @param netAllCap network's all capabilities alias
     * @return int32_t 0 for success, others are failed
     */
    int32_t GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap) override;

    /**
     * Set restrict background state
     *
     * @param isRestrictBackground restrict background state
     * @return int32_t 0 for success, others are failed
     */
    int32_t RestrictBackgroundChanged(bool isRestrictBackground);

    /**
     * Set airplane mode
     *
     * @param state airplane state
     * @return int32_t result
     */
    int32_t SetAirplaneMode(bool state) override;

    /**
     * Restore NetConn factory setting
     *
     * @return int32_t result
     */
    int32_t RestoreFactoryData() override;

private:
    enum RematchAllNetworksReason {
        REASON_NET_AVAILABLE_CHANGED,
        REASON_NET_CAPABILITIES_CHANGED,
        REASON_NET_LINK_INFO_CHANGED,
        REASON_NET_SCORE_CHANGED
    };
    void RematchAllNetworks(RematchAllNetworksReason reason);
    sptr<NetSupplier> GetBestNetworkForRequest(const sptr<NetRequest> &request);

private:
    void OnNetAvailableChanged(uint32_t supplierId, bool available) override;

    void OnNetCapabilitiesChanged(uint32_t supplierId, const NetAllCapabilities &allCaps) override;

    void OnNetLinkInfoChanged(uint32_t supplierId, const NetLinkInfo &linkInfo) override;

    void OnNetDetectionResultChanged(uint32_t netId, NetDetectionResultCode detectionResult,
                                     const std::string &urlRedirect) override;

    void OnNetScoreChanged(uint32_t supplierId, uint32_t score) override;

private:
    void OnStart() override;
    void OnStop() override;
    int32_t SystemReady() override;

private:
    sptr<NetSupplier> CreateNetSupplier(NetBearType bearType, const std::string &ident, const std::set<NetCap> &caps);
    sptr<NetSupplier> FindNetSupplier(uint32_t supplierId);
    sptr<NetSupplier> FindNetSupplierByNetId(uint32_t netId);
    std::list<sptr<NetSupplier>> FindNetSuppliersByInfo(NetBearType bearerType = BEARER_DEFAULT,
                                                        const std::string &ident = "");
    std::list<sptr<NetSupplier>> GetAvailableNetSuppliers() const;
    bool RemoveNetSupplier(int32_t supplierId);

    void CreateDefaultRequest();
    sptr<NetRequest> CreateNetRequest(sptr<NetSpecifier> netSpecifier, sptr<INetConnCallback> callback,
                                      uint32_t timeoutMs);
    sptr<NetRequest> FindNetRequest(uint32_t reqId);
    sptr<NetRequest> FindNetRequestByCallback(const sptr<INetConnCallback> &callback);
    std::list<sptr<NetRequest>> FindNetRequestsBySameSpecifier(const NetSpecifier &netSpecifier);
    std::list<sptr<NetRequest>> FindNetRequestsBySupplierId(uint32_t supplierId);
    bool RemoveNetRequest(const sptr<NetRequest> &request);

private:
    int32_t InvokeMethodSafety(const char *methodName, std::function<int32_t(void)> func);

private:
    void DumpSuppliersInfo();

private:
    std::map<uint32_t, sptr<NetSupplier>> netSuppliers_;
    std::map<uint32_t, sptr<NetRequest>> netRequests_;
    sptr<NetSupplier> defaultNetSupplier_;
    sptr<NetRequest> defaultNetRequest_;
    std::thread asyncThread_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SERVICE_H
