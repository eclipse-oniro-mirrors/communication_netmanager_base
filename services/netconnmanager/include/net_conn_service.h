/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "singleton.h"
#include "system_ability.h"

#include "http_proxy.h"
#include "net_activate.h"
#include "net_conn_event_handler.h"
#include "net_conn_service_iface.h"
#include "net_conn_service_stub.h"
#include "net_score.h"
#include "net_supplier.h"
#include "netsys_controller_callback.h"
#include "network.h"
#include "dns_result_call_back.h"
#include "net_factoryreset_callback.h"
#include "os_account_manager.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const int32_t PRIMARY_USER_ID = 100;
}

class NetConnService : public SystemAbility,
                       public INetActivateCallback,
                       public NetConnServiceStub,
                       public std::enable_shared_from_this<NetConnService> {
    DECLARE_SYSTEM_ABILITY(NetConnService)

    NetConnService();
    virtual ~NetConnService();
    using NET_SUPPLIER_MAP = std::map<uint32_t, sptr<NetSupplier>>;
    using NET_NETWORK_MAP = std::map<int32_t, std::shared_ptr<Network>>;
    using NET_ACTIVATE_MAP = std::map<uint32_t, std::shared_ptr<NetActivate>>;
    using NET_UIDREQUEST_MAP = std::map<uint32_t, uint32_t>;

public:
    static std::shared_ptr<NetConnService> &GetInstance()
    {
        static std::shared_ptr<NetConnService> instance = std::make_shared<NetConnService>();
        return instance;
    }
    void OnStart() override;
    void OnStop() override;
    /**
     * The interface in NetConnService can be called when the system is ready
     *
     * @return Returns 0, the system is ready, otherwise the system is not ready
     */
    int32_t SystemReady() override;

    /**
     * Disallow or allow a app to create AF_INET or AF_INET6 socket
     *
     * @param uid App's uid which need to be disallowed ot allowed to create AF_INET or AF_INET6 socket
     * @param allow 0 means disallow, 1 means allow
     * @return return 0 if OK, return error number if not OK
     */
    int32_t SetInternetPermission(uint32_t uid, uint8_t allow) override;

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
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> callback) override;

    /**
     * Register net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     * @param timeoutMS net connection time out
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> callback,
                                    const uint32_t &timeoutMS) override;

    /**
     * Request net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     * @param timeoutMS net connection time out
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RequestNetConnection(const sptr<NetSpecifier> netSpecifier, const sptr<INetConnCallback> callback,
                                    const uint32_t timeoutMS) override;
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
     * The interface is to get all iface and ident maps
     *
     * @param bearerType the type of network
     * @param ifaceNameIdentMaps the map of ifaceName and ident
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetIfaceNameIdentMaps(NetBearType bearerType,
                                  std::unordered_map<std::string, std::string> &ifaceNameIdentMaps) override;

    /**
     * register network detection return result method
     *
     * @param netId  Network ID
     * @param callback The callback of INetDetectionCallback interface
     * @return int32_t  Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;

    /**
     * unregister network detection return result method
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
    int32_t GetDefaultNet(int32_t &netId) override;
    int32_t HasDefaultNet(bool &flag) override;
    int32_t GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList) override;
    int32_t GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr) override;
    int32_t GetSpecificNet(NetBearType bearerType, std::list<int32_t> &netIdList) override;
    int32_t GetAllNets(std::list<int32_t> &netIdList) override;
    int32_t GetSpecificUidNet(int32_t uid, int32_t &netId) override;
    int32_t GetConnectionProperties(int32_t netId, NetLinkInfo &info) override;
    int32_t GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap) override;
    int32_t BindSocket(int32_t socketFd, int32_t netId) override;
    void HandleDetectionResult(uint32_t supplierId, NetDetectionStatus netState);
    int32_t RestrictBackgroundChanged(bool isRestrictBackground);
    /**
     * Set airplane mode
     *
     * @param state airplane state
     * @return Returns 0, successfully set airplane mode, otherwise it will fail
     */
    int32_t SetAirplaneMode(bool state) override;
    /**
     * Dump
     *
     * @param fd file description
     * @param args unused
     * @return Returns 0, successfully get dump info, otherwise it will fail
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;
    /**
     * Is default network metered
     *
     * @param save the metered state
     * @return Returns 0, Successfully get whether the default network is metered, otherwise it will fail
     */
    int32_t IsDefaultNetMetered(bool &isMetered) override;

    /**
     * Set http proxy server
     *
     * @param httpProxy the http proxy server
     * @return NETMANAGER_SUCCESS if OK, NET_CONN_ERR_HTTP_PROXY_INVALID if httpProxy is null string
     */
    int32_t SetGlobalHttpProxy(const HttpProxy &httpProxy) override;

    /**
     * Get http proxy server
     *
     * @param httpProxy output param, the http proxy server
     * @return NETMANAGER_SUCCESS if OK, NET_CONN_ERR_NO_HTTP_PROXY if httpProxy is null string
     */
    int32_t GetGlobalHttpProxy(HttpProxy &httpProxy) override;

    /**
     * Obtains the default proxy settings.
     *
     * <p>If a global proxy is set, the global proxy parameters are returned.
     * If the process is bound to a network using {@link setAppNet},
     * the {@link Network} proxy settings are returned.
     * In other cases, the default proxy settings of network are returned.
     *
     * @param bindNetId App bound network ID
     * @param httpProxy output param, the http proxy server
     * @return Returns NETMANAGER_SUCCESS even if HttpProxy is empty
     */
    int32_t GetDefaultHttpProxy(int32_t bindNetId, HttpProxy &httpProxy) override;

    /**
     * Get net id by identifier
     *
     * @param ident Net identifier
     * @param netIdList output param, the net id list
     * @return NETMANAGER_SUCCESS if OK, ERR_NO_NET_IDENT if ident is null string
     */
    int32_t GetNetIdByIdentifier(const std::string &ident, std::list<int32_t> &netIdList) override;

    /**
     * Activate network timeout
     *
     * @param reqId Net request id
     */
    void OnNetActivateTimeOut(uint32_t reqId) override;

    /**
     * The interface of network detection called when DNS health check failed
     *
     * @param netId network ID
     * @return int32_t Whether the network probe is successful
     */
    int32_t NetDetectionForDnsHealth(int32_t netId, bool dnsHealthSuccess);

    int32_t SetAppNet(int32_t netId) override;
    int32_t RegisterNetInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback) override;
    int32_t GetNetInterfaceConfiguration(const std::string &iface, NetInterfaceConfiguration &config) override;
    int32_t AddNetworkRoute(int32_t netId, const std::string &ifName,
                            const std::string &destination, const std::string &nextHop) override;
    int32_t RemoveNetworkRoute(int32_t netId, const std::string &ifName,
                               const std::string &destination, const std::string &nextHop) override;
    int32_t AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                int32_t prefixLength) override;
    int32_t DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                int32_t prefixLength) override;
    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName) override;
    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                         const std::string &ifName) override;
    int32_t RegisterSlotType(uint32_t supplierId, int32_t type) override;
    int32_t GetSlotType(std::string &type) override;
    int32_t FactoryResetNetwork() override;
    int32_t RegisterNetFactoryResetCallback(const sptr<INetFactoryResetCallback> &callback) override;
    int32_t IsPreferCellularUrl(const std::string& url, bool& preferCellular) override;
    int32_t RegisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback) override;
    int32_t UnregisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback) override;
    bool IsAddrInOtherNetwork(const std::string &ifaceName, int32_t netId, const INetAddr &netAddr);
    bool IsIfaceNameInUse(const std::string &ifaceName, int32_t netId);
    int32_t UpdateSupplierScore(NetBearType bearerType, bool isBetter) override;

private:
    class NetInterfaceStateCallback : public NetsysControllerCallback {
    public:
        NetInterfaceStateCallback() = default;
        ~NetInterfaceStateCallback() = default;
        int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAdded(const std::string &iface) override;
        int32_t OnInterfaceRemoved(const std::string &iface) override;
        int32_t OnInterfaceChanged(const std::string &iface, bool up) override;
        int32_t OnInterfaceLinkStateChanged(const std::string &iface, bool up) override;
        int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                               const std::string &ifName) override;
        int32_t OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult) override;
        int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override;

        int32_t RegisterInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback);

    private:
        std::mutex mutex_;
        std::vector<sptr<INetInterfaceStateCallback>> ifaceStateCallbacks_;
    };

protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    enum RegisterType {
        INVALIDTYPE,
        REGISTER,
        REQUEST,
    };
    bool Init();
    void RecoverInfo();
    std::list<sptr<NetSupplier>> GetNetSupplierFromList(NetBearType bearerType, const std::string &ident = "");
    sptr<NetSupplier> GetNetSupplierFromList(NetBearType bearerType, const std::string &ident,
                                             const std::set<NetCap> &netCaps);
    int32_t ActivateNetwork(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                            const uint32_t &timeoutMS);
    void CallbackForSupplier(sptr<NetSupplier> &supplier, CallbackType type);
    void CallbackForAvailable(sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback);
    uint32_t FindBestNetworkForRequest(sptr<NetSupplier> &supplier, std::shared_ptr<NetActivate> &netActivateNetwork);
    uint32_t FindInternalNetworkForRequest(std::shared_ptr<NetActivate> &netActivateNetwork,
                                           sptr<NetSupplier> &supplier);
    void SendRequestToAllNetwork(std::shared_ptr<NetActivate> request);
    void SendBestScoreAllNetwork(uint32_t reqId, int32_t bestScore, uint32_t supplierId);
    void SendAllRequestToNetwork(sptr<NetSupplier> supplier);
    void FindBestNetworkForAllRequest();
    void MakeDefaultNetWork(sptr<NetSupplier> &oldService, sptr<NetSupplier> &newService);
    void NotFindBestSupplier(uint32_t reqId, const std::shared_ptr<NetActivate> &active,
                             const sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback);
    void CreateDefaultRequest();
    int32_t RegUnRegNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback, bool isReg);
    int32_t GenerateNetId();
    int32_t GenerateInternalNetId();
    bool FindSameCallback(const sptr<INetConnCallback> &callback, uint32_t &reqId);
    bool FindSameCallback(const sptr<INetConnCallback> &callback, uint32_t &reqId, RegisterType &registerType);
    void GetDumpMessage(std::string &message);
    sptr<NetSupplier> FindNetSupplier(uint32_t supplierId);
    int32_t RegisterNetSupplierAsync(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps,
                                     uint32_t &supplierId);
    int32_t UnregisterNetSupplierAsync(uint32_t supplierId);
    int32_t RegisterNetSupplierCallbackAsync(uint32_t supplierId, const sptr<INetSupplierCallback> &callback);
    int32_t RegisterNetConnCallbackAsync(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                         const uint32_t &timeoutMS, const uint32_t callingUid);
    int32_t RequestNetConnectionAsync(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                         const uint32_t &timeoutMS, const uint32_t callingUid);
    int32_t UnregisterNetConnCallbackAsync(const sptr<INetConnCallback> &callback, const uint32_t callingUid);
    int32_t RegUnRegNetDetectionCallbackAsync(int32_t netId, const sptr<INetDetectionCallback> &callback, bool isReg);
    int32_t UpdateNetStateForTestAsync(const sptr<NetSpecifier> &netSpecifier, int32_t netState);
    int32_t UpdateNetSupplierInfoAsync(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo);
    int32_t UpdateNetLinkInfoAsync(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo);
    int32_t NetDetectionAsync(int32_t netId);
    int32_t RestrictBackgroundChangedAsync(bool restrictBackground);
    int32_t UpdateSupplierScoreAsync(NetBearType bearerType, bool isBetter);
    void SendHttpProxyChangeBroadcast(const HttpProxy &httpProxy);
    void RequestAllNetworkExceptDefault();
    void LoadGlobalHttpProxy();
    void UpdateGlobalHttpProxy(const HttpProxy &httpProxy);
    void ActiveHttpProxy();
    void DecreaseNetConnCallbackCntForUid(const uint32_t callingUid,
        const RegisterType registerType = REGISTER);
    int32_t IncreaseNetConnCallbackCntForUid(const uint32_t callingUid,
        const RegisterType registerType = REGISTER);

    void OnNetSysRestart();

    bool IsSupplierMatchRequestAndNetwork(sptr<NetSupplier> ns);
    std::vector<std::string> GetPreferredUrl();
    bool IsValidDecValue(const std::string &inputValue);
    int32_t GetDelayNotifyTime();
    int32_t NetDetectionForDnsHealthSync(int32_t netId, bool dnsHealthSuccess);
    std::vector<sptr<NetSupplier>> FindSupplierWithInternetByBearerType(NetBearType bearerType);
    int32_t GetCallingUserId(int32_t &userId);
    inline bool IsPrimaryUserId(const int32_t userId)
    {
        return userId == PRIMARY_USER_ID;
    }

    // for NET_CAPABILITY_INTERNAL_DEFAULT
    bool IsInRequestNetUids(int32_t uid);
private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool registerToService_;
    ServiceRunningState state_;
    sptr<NetSpecifier> defaultNetSpecifier_ = nullptr;
    std::shared_ptr<NetActivate> defaultNetActivate_ = nullptr;
    sptr<NetSupplier> defaultNetSupplier_ = nullptr;
    NET_SUPPLIER_MAP netSuppliers_;
    NET_ACTIVATE_MAP netActivates_;
    NET_UIDREQUEST_MAP netUidRequest_;
    NET_UIDREQUEST_MAP internalDefaultUidRequest_;
    NET_NETWORK_MAP networks_;
    sptr<NetConnServiceIface> serviceIface_ = nullptr;
    std::atomic<int32_t> netIdLastValue_ = MIN_NET_ID - 1;
    std::atomic<int32_t> internalNetIdLastValue_ = MIN_INTERNAL_NET_ID;
    std::atomic<bool> isGlobalProxyLoaded_ = false;
    HttpProxy globalHttpProxy_;
    std::mutex globalHttpProxyMutex_;
    std::mutex netManagerMutex_;
    std::shared_ptr<AppExecFwk::EventRunner> netConnEventRunner_ = nullptr;
    std::shared_ptr<NetConnEventHandler> netConnEventHandler_ = nullptr;
    std::shared_ptr<AppExecFwk::EventRunner> netActEventRunner_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> netActEventHandler_ = nullptr;
    sptr<NetInterfaceStateCallback> interfaceStateCallback_ = nullptr;
    sptr<NetDnsResultCallback> dnsResultCallback_ = nullptr;
    sptr<NetFactoryResetCallback> netFactoryResetCallback_ = nullptr;
    std::atomic_bool httpProxyThreadNeedRun_ = false;
    std::condition_variable httpProxyThreadCv_;
    std::mutex httpProxyThreadMutex_;
    static constexpr const uint32_t HTTP_PROXY_ACTIVE_PERIOD_S = 120;
    std::map<int32_t, sptr<IPreAirplaneCallback>> preAirplaneCallbacks_;
    std::mutex preAirplaneCbsMutex_;

    bool hasSARemoved_ = false;

private:
    class ConnCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ConnCallbackDeathRecipient(NetConnService &client) : client_(client) {}
        ~ConnCallbackDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetConnService &client_;
    };
    void OnRemoteDied(const wptr<IRemoteObject> &remoteObject);
    void AddClientDeathRecipient(const sptr<INetConnCallback> &callback);
    void RemoveClientDeathRecipient(const sptr<INetConnCallback> &callback);
    void RemoveALLClientDeathRecipient();
    std::mutex remoteMutex_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::vector<sptr<INetConnCallback>> remoteCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SERVICE_H
