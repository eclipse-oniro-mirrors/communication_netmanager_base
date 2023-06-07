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
#include "i_net_supplier_callback.h"
#include "net_handle.h"
#include "net_link_info.h"
#include "net_specifier.h"
#include "net_supplier_callback_base.h"

namespace OHOS {
namespace nmd {
class FwmarkClient;
}
namespace NetManagerStandard {
class NetConnClient : public Singleton<NetConnClient> {
public:
    NetConnClient();
    ~NetConnClient();

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
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback);

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
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                    const uint32_t &timeoutMS);

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
     * The interface is to bind socket
     *
     * @param socket_fd socket file description
     * @param netId network id
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t BindSocket(int32_t socket_fd, int32_t netId);

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
     * Set iface up
     *
     * @param ifaceName Network port device name
     * @return Returns 0 success. Otherwise fail.
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t InterfaceSetIffUp(const std::string &ifaceName);

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
    sptr<INetConnService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetConnService> NetConnService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::map<uint32_t, sptr<INetSupplierCallback>> netSupplierCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NET_CONN_MANAGER_H
