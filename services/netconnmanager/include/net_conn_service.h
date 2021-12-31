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

#ifndef NET_CONN_SERVICE_H
#define NET_CONN_SERVICE_H

#include <list>
#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include "singleton.h"
#include "system_ability.h"

#include "net_conn_service_stub.h"
#include "net_conn_service_iface.h"
#include "net_supplier.h"
#include "net_activate.h"
#include "network.h"
#include "net_score.h"
#include "timer.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnService : public SystemAbility,
    public NetConnServiceStub,
    public std::enable_shared_from_this<NetConnService> {
    DECLARE_DELAYED_SINGLETON(NetConnService)
    DECLARE_SYSTEM_ABILITY(NetConnService)

    using NET_SUPPLIER_MAP = std::map<uint32_t, sptr<NetSupplier>>;
    using NET_NETWORK_MAP = std::map<int32_t, sptr<Network>>;
    using NET_ACTIVATE_MAP = std::map<uint32_t, sptr<NetActivate>>;

public:
    void OnStart() override;
    void OnStop() override;
    /**
     * @brief The interface in NetConnService can be called when the system is ready
     *
     * @return Returns 0, the system is ready, otherwise the system is not ready
     */
    int32_t SystemReady() override;

    /**
     * @brief The interface is register the network
     *
     * @param netType Network Type
     * @param ident Unique identification of mobile phone card
     * @param netCapabilities Network capabilities registered by the network supplier
     * @param supplierId out param, return supplier id
     *
     * @return function result
     */
    int32_t RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities,
        uint32_t &supplierId) override;

    /**
     * @brief The interface is unregister the network
     *
     * @param supplierId The id of the network supplier
     *
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t UnregisterNetSupplier(uint32_t supplierId) override;

    /**
     * @brief Register supplier callback
     *
     * @param supplierId The id of the network supplier
     * @param callback INetSupplierCallback callback interface
     *
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback) override;

     /**
     * @brief Register net connection callback
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback) override;

    /**
     * @brief Register net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) override;

    /**
     * @brief Unregister net connection callback
     *
     * @return Returns 0, successfully unregister net connection callback, otherwise it will fail
     */
    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback) override;

    /**
     * @brief Unregister net connection callback by NetSpecifier
     *
     * @return Returns 0, successfully unregister net connection callback, otherwise it will fail
     */
    int32_t UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) override;

    int32_t UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState) override;
    /**
     * @brief The interface is update network connection status information
     *
     * @param supplierId The id of the network supplier
     * @param netSupplierInfo network connection status information
     *
     * @return Returns 0, successfully update the network connection status information, otherwise it will fail
     */
    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) override;

    /**
     * @brief The interface is Create or delete network services based on the supplierId and the netCapabilities
     *
     * @param supplierId The id of the network supplier
     * @param netCapabilities Network capabilities registered by the network supplier
     *
     * @return Returns 0, successfully create network service or delete network service, otherwise fail
     */
    int32_t UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities) override;

    /**
     * @brief The interface is update network link attribute information
     *
     * @param supplierId The id of the network supplier
     * @param netLinkInfo network link attribute information
     *
     * @return Returns 0, successfully update the network link attribute information, otherwise it will fail
     */
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) override;

    /**
     * @brief The interface is get the iface name for network
     *
     * @param netType Network Type
     * @param ident Unique identification of mobile phone card
     * @param ifaceName save the obtained ifaceName
     * @return Returns 0, successfully get the network link attribute iface name, otherwise it will fail
     */
    int32_t GetIfaceNameByType(uint32_t netType, const std::string &ident, std::string &ifaceName) override;

    /**
     * @brief register network detection return result method
     *
     * @param netId  Network ID
     * @param callback The callback of INetDetectionCallback interface
     * @return int32_t  Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;

    /**
     * @brief unregister network detection return result method
     *
     * @param netId Network ID
     * @param callback  The callback of INetDetectionCallback interface
     * @return int32_t  Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;

    /**
     * @brief The interface of network detection called by the application
     *
     * @param netId network ID
     * @return int32_t Whether the network probe is successful
     */
    int32_t NetDetection(int32_t netId) override;

    int32_t ActivateNetwork(const sptr<NetSpecifier> &netSpecifier,
                            const sptr<INetConnCallback> &callback,
                            uint32_t &reqId) override;
    int32_t  DeactivateNetwork(uint32_t &reqId) override;

    int32_t GetDefaultNet(int32_t &netId) override;
    int32_t HasDefaultNet(bool &flag) override;
    int32_t GetAddressesByName(const std::string &host, int32_t netId, std::list<INetAddr> &addrList) override;
    int32_t GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr) override;
    int32_t GetSpecificNet(uint32_t type, std::list<int32_t> &netIdList) override;
    int32_t GetAllNets(std::list<int32_t> &netIdList) override;
    int32_t GetSpecificUidNet(int32_t uid, int32_t &netId) override;
    int32_t GetConnectionProperties(int32_t netId, NetLinkInfo &info) override;
    int32_t GetNetCapabilities(int32_t netId, uint64_t &cap) override;
    int32_t BindSocket(int32_t socket_fd, int32_t netId) override;
    void HandleDetectionResult(uint32_t supplierId, bool ifValid);
    /**
     * @brief Set airplane mode
     *
     * @param state airplane state
     * @return int32_t result
     */
    int32_t SetAirplaneMode(bool state) override;
    int32_t ResetNetConnService();

private:
    bool Init();
    sptr<NetSupplier> GetNetSupplierFromList(uint32_t netType, const std::string &ident);
    sptr<NetSupplier> GetNetSupplierFromList(uint32_t netType, const std::string &ident, uint64_t capabilities);
    void ThreadExitTask();
    int32_t NotifyNetConnStateChanged(const sptr<NetConnCallbackInfo> &info);
    void CallbackForNetwork(sptr<Network>& network, CallbackType type);
    void CallbackForSupplier(sptr<NetSupplier>& supplier, CallbackType type);
    uint32_t FindBestNetworkForRequest(sptr<NetSupplier>& supplier, sptr<NetActivate>& netActivateNetwork);
    void SendRequestToAllNetwork(sptr<NetActivate> request);
    void SendBestScoreAllNetwork(uint32_t reqId, int32_t bestScore);
    void SendAllRequestToNetwork(sptr<NetSupplier> supplier);
    void RegisterSupplierCallback(sptr<NetSupplier> supplier, uint32_t netType, uint64_t netCapabilities);
    void FindBestNetworkForAllRequest();
    void MakeDefaultNetWork(sptr<NetSupplier>& oldService, sptr<NetSupplier>& newService);
    void NotFindBestSupplier(uint32_t reqId, const sptr<NetActivate> &active,
        const sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback);
    void CreateDefaultRequest();
    sptr<Network> FindExitNetworkTestNetDetection();
    int32_t RegUnRegNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback, bool isReg);
    int32_t GenerateNetId();

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool registerToService_;
    ServiceRunningState state_;
    sptr<NetActivate> defaultNetActivate_ = nullptr;
    sptr<NetSupplier> defaultNetSupplier_ = nullptr;
    NET_SUPPLIER_MAP netSuppliers_;
    NET_ACTIVATE_MAP netActivates_;
    NET_NETWORK_MAP networks_;
    std::unique_ptr<NetScore> netScore_ = nullptr;
    NetSpecifier specAll_;
    std::map<NetSpecifier, std::vector<sptr<INetConnCallback>>> netConnCallback_;
    bool isNeedTestNetDetection_ = true;
    sptr<NetConnServiceIface> serviceIface_ = nullptr;
    std::atomic<int32_t> netIdLastValue_ = MIN_NET_ID - 1;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SERVICE_H