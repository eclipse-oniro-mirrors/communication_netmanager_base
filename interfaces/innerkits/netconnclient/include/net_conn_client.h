/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_MANAGER_H
#define NET_CONN_MANAGER_H

#include <map>
#include <string>

#include "parcel.h"
#include "singleton.h"

#include "http_proxy.h"
#include "i_net_conn_service.h"
#include "i_net_interface_callback.h"
#include "i_net_supplier_callback.h"
#include "net_handle.h"
#include "net_link_info.h"
#include "net_specifier.h"
#include "net_supplier_callback_base.h"
#include "i_net_factoryreset_callback.h"
#include "safe_map.h"

namespace OHOS {
namespace nmd {
class FwmarkClient;
}
namespace NetManagerStandard {
constexpr uint32_t RESERVED_BUFFER_SIZE = 512;
class NetConnClient {
public:
    static NetConnClient &GetInstance();

    /**
     * The interface in NetConnService can be called when the system is ready
     *
     * @return Returns 0, the system is ready, otherwise the system is not ready
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SystemReady();

    /**
     * The interface is set permission for network
     *
     * @param The specified UID of app
     * @param allow internet permission
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetInternetPermission(uint32_t uid, uint8_t allow);

    /**
     * The interface is register the network
     *
     * @param bearerType Bearer Network Type
     * @param ident Unique identification of mobile phone card
     * @param netCaps Network capabilities registered by the network supplier
     * @param supplierId out param, return supplier id
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t RegisterNetSupplier(NetBearType bearerType, const std::string &ident, const std::set<NetCap> &netCaps,
                                uint32_t &supplierId);

    /**
     * The interface is unregister the network
     *
     * @param supplierId The id of the network supplier
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnregisterNetSupplier(uint32_t supplierId);

    /**
     * Register supplier callback
     *
     * @param supplierId The id of the network supplier
     * @param callback INetSupplierCallback callback interface
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterNetSupplierCallback(uint32_t supplierId, const sptr<NetSupplierCallbackBase> &callback);

    /**
     * The interface is update network connection status information
     *
     * @param supplierId The id of the network supplier
     * @param netSupplierInfo network connection status information
     * @return Returns 0, successfully update the network connection status information, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo);

    /**
     * The interface is update network link attribute information
     *
     * @param supplierId The id of the network supplier
     * @param netLinkInfo network link attribute information
     * @return Returns 0, successfully update the network link attribute information, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo);

    /**
     * Register net connection callback
     *
     * @param callback The callback of INetConnCallback interface
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> callback);

    /**
     * Register net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     * @param timeoutMS net connection time out
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> callback,
                                    const uint32_t &timeoutMS);

    /**
     * Request net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     * @param timeoutMS net connection time out
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RequestNetConnection(const sptr<NetSpecifier> netSpecifier, const sptr<INetConnCallback> callback,
                                    const uint32_t timeoutMS);
    /**
     * Unregister net connection callback
     *
     * @param callback The callback of INetConnCallback interface
     * @return Returns 0, successfully unregister net connection callback, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback);

    /**
     * Register net detection callback by netId
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetDetectionCallback interface
     * @param timeoutMS net connection time out
     * @return Returns 0, successfully register net detection callback, otherwise it will failed
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback);
    /**
     * Unregister net detection callback by netId
     *
     * @param callback The callback of INetDetectionCallback interface
     * @return Returns 0, successfully unregister net detection callback, otherwise it will fail
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback);
    
    /**
     * The interface is to get default network
     *
     * @param netHandle network handle
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetDefaultNet(NetHandle &netHandle);

    /**
     * The interface is to check whether has default network
     *
     * @param flag has default network or not
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t HasDefaultNet(bool &flag);

    /**
     * The interface is to get all acvite network
     *
     * @param netList a list of network
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetAllNets(std::list<sptr<NetHandle>> &netList);

    /**
     * get the network link information of the connection
     *
     * @param netHandle network handle
     * @param info network link infomation
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetConnectionProperties(const NetHandle &netHandle, NetLinkInfo &info);

    /**
     * get all capabilities from network
     *
     * @param netHandle network handle
     * @param netAllCap network all of capabilities
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetNetCapabilities(const NetHandle &netHandle, NetAllCapabilities &netAllCap);

    /**
     * The interface is to get addresses by network name
     *
     * @param host domain name
     * @param netId network id
     * @param addrList list of network addresses
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList);

    /**
     * The interface is to get address by network name
     *
     * @param host domain name
     * @param netId network
     * @param addr network address
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr);

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
                                 std::unordered_map<std::string, std::string> &ifaceNameIdentMaps);

    /**
     * The interface is to bind socket
     *
     * @param socketFd socket file description
     * @param netId network id
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t BindSocket(int32_t socketFd, int32_t netId);

    /**
     * The interface of network detection called by the application
     *
     * @param netHandle network handle
     * @return int32_t Whether the network probe is successful
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t NetDetection(const NetHandle &netHandle);

    /**
     * set air plane mode on or off
     *
     * @param state air plane mode on or not
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetAirplaneMode(bool state);

    /**
     * check whether the network meter is default
     *
     * @param isMetered the network meter is default or not
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t IsDefaultNetMetered(bool &isMetered);

    /**
     * set global http proxy in the network
     *
     * @param httpProxy http proxy
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetGlobalHttpProxy(const HttpProxy &httpProxy);

    /**
     * get global http proxy in the network
     *
     * @param httpProxy http proxy
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetGlobalHttpProxy(HttpProxy &httpProxy);

    /**
     * set network id of app binding network
     *
     * @param netId network id
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetDefaultHttpProxy(HttpProxy &httpProxy);

    /**
     * set network id of app binding network
     *
     * @param netId network id
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetAppNet(int32_t netId);

    /**
     * get network id of app binding network
     *
     * @param netId network id
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetAppNet(int32_t &netId);

    /**
     * Get network id by identifier
     *
     * @param ident identifier
     * @param netIdList  list of network id
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetNetIdByIdentifier(const std::string &ident, std::list<int32_t> &netIdList);

    /**
     * Register network interface state change callback
     *
     * @param callback The callback of INetInterfaceStateCallback interface
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterNetInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback);

    /**
     * Get network interface configuration
     *
     * @param ifaceName Network port device name
     * @param config Network interface configuration
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetNetInterfaceConfiguration(const std::string &iface, NetInterfaceConfiguration &config);

    int32_t AddNetworkRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                            const std::string &nextHop);
    int32_t RemoveNetworkRoute(int32_t netId, const std::string &ifName, const std::string &destination,
                               const std::string &nextHop);
    int32_t AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                int32_t prefixLength);
    int32_t DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                int32_t prefixLength);
    int32_t AddStaticArp(const std::string &ipAddr, const std::string &macAddr, const std::string &ifName);
    int32_t DelStaticArp(const std::string &ipAddr, const std::string &macAddr, const std::string &ifName);
    int32_t GetPinSetForHostName(const std::string &hostname, std::string &pins);
    int32_t GetTrustAnchorsForHostName(const std::string &hostname, std::vector<std::string> &certs);

    int32_t RegisterSlotType(uint32_t supplierId, int32_t type);
    int32_t GetSlotType(std::string &type);

    int32_t FactoryResetNetwork();
    int32_t RegisterNetFactoryResetCallback(const sptr<INetFactoryResetCallback> &callback);
    void RegisterAppHttpProxyCallback(std::function<void(const HttpProxy &httpProxy)> callback, uint32_t &callbackid);
    void UnregisterAppHttpProxyCallback(uint32_t callbackid);
    int32_t SetAppHttpProxy(const HttpProxy &httpProxy);
     /**
     * Whether this url prefer cellular
     *
     * @param url url input
     * @param preferCellular out param, whether prefer cellular
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t IsPreferCellularUrl(const std::string& url, bool& preferCellular);

    int32_t RegisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback);

    int32_t UnregisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback);

    int32_t UpdateSupplierScore(NetBearType bearerType, bool isBetter);

private:
    class NetConnDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetConnDeathRecipient(NetConnClient &client) : client_(client) {}
        ~NetConnDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetConnClient &client_;
    };

private:
    NetConnClient();
    ~NetConnClient();
    NetConnClient& operator=(const NetConnClient&) = delete;
    NetConnClient(const NetConnClient&) = delete;

    sptr<INetConnService> GetProxy();
    void RecoverCallbackAndGlobalProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex appHttpProxyCbMapMutex_;
    uint32_t currentCallbackId_ = 0;
    std::map<uint32_t, std::function<void(const HttpProxy &httpProxy)>> appHttpProxyCbMap_;
    HttpProxy appHttpProxy_;
    HttpProxy globalHttpProxy_;
    char buffer_[RESERVED_BUFFER_SIZE] = {0};
    std::mutex mutex_;
    sptr<INetConnService> NetConnService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::map<uint32_t, sptr<INetSupplierCallback>> netSupplierCallback_;
    std::list<std::tuple<sptr<NetSpecifier>, sptr<INetConnCallback>, uint32_t>> registerConnTupleList_;
    SafeMap<uint32_t, uint8_t> netPermissionMap_;
    sptr<IPreAirplaneCallback> preAirplaneCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NET_CONN_MANAGER_H
