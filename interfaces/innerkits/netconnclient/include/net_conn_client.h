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

#ifndef NET_CONN_MANAGER_H
#define NET_CONN_MANAGER_H

#include <string>
#include <map>

#include "parcel.h"
#include "singleton.h"

#include "i_net_conn_service.h"
#include "i_net_supplier_callback.h"
#include "net_supplier_callback_base.h"
#include "net_link_info.h"
#include "net_specifier.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnClient {
    DECLARE_DELAYED_SINGLETON(NetConnClient)

public:
    int32_t SystemReady();
    int32_t RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities);
    int32_t RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities,
        uint32_t &supplierId);
    int32_t UnregisterNetSupplier(uint32_t supplierId);
    int32_t RegisterNetSupplierCallback(uint32_t supplierId, const sptr<NetSupplierCallbackBase> &callback);
    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo);
    int32_t UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities);
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo);

    int32_t ActivateNetwork(
        const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback, uint32_t &reqId);
    int32_t DeactivateNetwork(uint32_t &reqId);
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback);
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback);
    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback);
    int32_t UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback);
    int32_t UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState);
    int32_t GetDefaultNet(int32_t &netId);
    int32_t GetSpecificNet(uint32_t type, std::list<int32_t> &netIdList);
    int32_t GetAllNets(std::list<int32_t> &netIdList);
    int32_t GetSpecificUidNet(int32_t uid, int32_t &netId);
    int32_t GetConnectionProperties(int32_t netId, NetLinkInfo &info);
    int32_t GetNetCapabilities(int32_t netId, uint64_t &cap);
    int32_t GetIfaceNameByType(uint32_t netType, const std::string &ident, std::string &ifaceName);
    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback);
    int32_t UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback);
    int32_t NetDetection(int32_t netId);
    int32_t SetAirplaneMode(bool state);

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