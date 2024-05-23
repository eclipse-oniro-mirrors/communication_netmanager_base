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

#ifndef NET_CONN_SERVICE_STUB_H
#define NET_CONN_SERVICE_STUB_H

#include <map>

#include "iremote_stub.h"

#include "i_net_conn_service.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnServiceStub : public IRemoteStub<INetConnService> {
public:
    NetConnServiceStub();
    ~NetConnServiceStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using NetConnServiceFunc = int32_t (NetConnServiceStub::*)(MessageParcel &, MessageParcel &);
    using NetConnServiceFuncPer = std::pair<NetConnServiceFunc, std::set<std::string>>;

private:
    void InitQueryFuncToInterfaceMap();
    void InitStaticArpToInterfaceMap();
    void InitInterfaceFuncToInterfaceMap();
    void InitResetNetFuncToInterfaceMap();
    void InitAll();
    bool CheckPermission(const std::set<std::string> &permissions);
    bool CheckPermissionWithCache(const std::set<std::string> &permissions);
    int32_t OnRequestCheck(uint32_t code, const std::set<std::string> &permissions);
    int32_t OnSystemReady(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetInternetPermission(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetSupplier(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnregisterNetSupplier(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetSupplierCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetConnCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetConnCallbackBySpecifier(MessageParcel &data, MessageParcel &reply);
    int32_t OnRequestNetConnectionBySpecifier(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnregisterNetConnCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateNetStateForTest(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateNetSupplierInfo(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateNetLinkInfo(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDefaultNet(MessageParcel &data, MessageParcel &reply);
    int32_t OnHasDefaultNet(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetIfaceNames(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetIfaceNameByType(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetIfaceNameIdentMaps(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetDetectionCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnRegisterNetDetectionCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnNetDetection(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetSpecificNet(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetAllNets(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetSpecificUidNet(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetConnectionProperties(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetNetCapabilities(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetAddressesByName(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetAddressByName(MessageParcel &data, MessageParcel &reply);
    int32_t OnBindSocket(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetAirplaneMode(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsDefaultNetMetered(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetGlobalHttpProxy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetGlobalHttpProxy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDefaultHttpProxy(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetNetIdByIdentifier(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetAppNet(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetInterfaceCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetNetInterfaceConfiguration(MessageParcel &data, MessageParcel &reply);
    int32_t OnAddNetworkRoute(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoveNetworkRoute(MessageParcel &data, MessageParcel &reply);
    int32_t OnAddInterfaceAddress(MessageParcel &data, MessageParcel &reply);
    int32_t OnDelInterfaceAddress(MessageParcel &data, MessageParcel &reply);
    int32_t OnAddStaticArp(MessageParcel &data, MessageParcel &reply);
    int32_t OnDelStaticArp(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterSlotType(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetSlotType(MessageParcel &data, MessageParcel &reply);
    int32_t OnFactoryResetNetwork(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterNetFactoryResetCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsPreferCellularUrl(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterPreAirplaneCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnregisterPreAirplaneCallback(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateSupplierScore(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, NetConnServiceFuncPer> memberFuncMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SERVICE_STUB_H
