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

#include "net_conn_service_stub.h"
#include "ipc_skeleton.h"
#include "net_conn_constants.h"
#include "net_conn_types.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_permission.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_IFACE_NUM = 16;
constexpr uint32_t MAX_NET_CAP_NUM = 32;
constexpr uint32_t UID_FOUNDATION = 5523;
constexpr uint32_t UID_BROKER_SERVICE = 5557;
const std::vector<uint32_t> SYSTEM_CODE{static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_AIRPLANE_MODE),
                                        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_GLOBAL_HTTP_PROXY),
                                        static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_GLOBAL_HTTP_PROXY)};
const std::vector<uint32_t> PERMISSION_NEED_CACHE_CODES{
    static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GETDEFAULTNETWORK),
    static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_HASDEFAULTNET)};
} // namespace
NetConnServiceStub::NetConnServiceStub()
{
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SYSTEM_READY)] = {
        &NetConnServiceStub::OnSystemReady, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK)] = {
        &NetConnServiceStub::OnRegisterNetConnCallback, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER)] = {
        &NetConnServiceStub::OnRegisterNetConnCallbackBySpecifier, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REQUEST_NET_CONNECTION)] = {
        &NetConnServiceStub::OnRequestNetConnectionBySpecifier, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_CONN_CALLBACK)] = {
        &NetConnServiceStub::OnUnregisterNetConnCallback, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UPDATE_NET_STATE_FOR_TEST)] = {
        &NetConnServiceStub::OnUpdateNetStateForTest, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REG_NET_SUPPLIER)] = {
        &NetConnServiceStub::OnRegisterNetSupplier, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREG_NETWORK)] = {
        &NetConnServiceStub::OnUnregisterNetSupplier, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_SUPPLIER_INFO)] = {
        &NetConnServiceStub::OnUpdateNetSupplierInfo, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_LINK_INFO)] = {
        &NetConnServiceStub::OnUpdateNetLinkInfo, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK)] = {
        &NetConnServiceStub::OnRegisterNetDetectionCallback, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK)] = {
        &NetConnServiceStub::OnUnRegisterNetDetectionCallback, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_NET_DETECTION)] = {
        &NetConnServiceStub::OnNetDetection, {Permission::GET_NETWORK_INFO, Permission::INTERNET}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_BIND_SOCKET)] = {&NetConnServiceStub::OnBindSocket,
                                                                                    {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK)] = {
        &NetConnServiceStub::OnRegisterNetSupplierCallback, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_AIRPLANE_MODE)] = {
        &NetConnServiceStub::OnSetAirplaneMode, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_GLOBAL_HTTP_PROXY)] = {
        &NetConnServiceStub::OnSetGlobalHttpProxy, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_APP_NET)] = {&NetConnServiceStub::OnSetAppNet,
                                                                                    {Permission::INTERNET}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_INTERNET_PERMISSION)] = {
        &NetConnServiceStub::OnSetInternetPermission, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_INTERFACE_CALLBACK)] = {
        &NetConnServiceStub::OnRegisterNetInterfaceCallback, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ROUTE)] = {
        &NetConnServiceStub::OnAddNetworkRoute, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ROUTE)] = {
        &NetConnServiceStub::OnRemoveNetworkRoute, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_IS_PREFER_CELLULAR_URL)] = {
        &NetConnServiceStub::OnIsPreferCellularUrl, {Permission::GET_NETWORK_INFO}};
    InitAll();
}

void NetConnServiceStub::InitAll()
{
    InitInterfaceFuncToInterfaceMap();
    InitResetNetFuncToInterfaceMap();
    InitStaticArpToInterfaceMap();
    InitQueryFuncToInterfaceMap();
}

void NetConnServiceStub::InitInterfaceFuncToInterfaceMap()
{
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS)] = {
        &NetConnServiceStub::OnAddInterfaceAddress, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ADDRESS)] = {
        &NetConnServiceStub::OnDelInterfaceAddress, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_PREAIRPLANE_CALLBACK)] = {
        &NetConnServiceStub::OnRegisterPreAirplaneCallback, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_PREAIRPLANE_CALLBACK)] = {
        &NetConnServiceStub::OnUnregisterPreAirplaneCallback, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UPDATE_SUPPLIER_SCORE)] = {
        &NetConnServiceStub::OnUpdateSupplierScore, {Permission::CONNECTIVITY_INTERNAL}};
}

void NetConnServiceStub::InitResetNetFuncToInterfaceMap()
{
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_FACTORYRESET_NETWORK)] = {
        &NetConnServiceStub::OnFactoryResetNetwork, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_FACTORYRESET_CALLBACK)] = {
        &NetConnServiceStub::OnRegisterNetFactoryResetCallback, {Permission::CONNECTIVITY_INTERNAL}};
}

void NetConnServiceStub::InitStaticArpToInterfaceMap()
{
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_STATIC_ARP)] = {
        &NetConnServiceStub::OnAddStaticArp, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_DEL_STATIC_ARP)] = {
        &NetConnServiceStub::OnDelStaticArp, {Permission::CONNECTIVITY_INTERNAL}};
}

void NetConnServiceStub::InitQueryFuncToInterfaceMap()
{
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACE_NAMES)] = {
        &NetConnServiceStub::OnGetIfaceNames, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACENAME_BY_TYPE)] = {
        &NetConnServiceStub::OnGetIfaceNameByType, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_GET_IFACENAME_IDENT_MAPS)] = {
        &NetConnServiceStub::OnGetIfaceNameIdentMaps, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GETDEFAULTNETWORK)] = {
        &NetConnServiceStub::OnGetDefaultNet, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_HASDEFAULTNET)] = {
        &NetConnServiceStub::OnHasDefaultNet, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_NET)] = {
        &NetConnServiceStub::OnGetSpecificNet, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ALL_NETS)] = {&NetConnServiceStub::OnGetAllNets,
                                                                                     {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_UID_NET)] = {
        &NetConnServiceStub::OnGetSpecificUidNet, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_CONNECTION_PROPERTIES)] = {
        &NetConnServiceStub::OnGetConnectionProperties, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_CAPABILITIES)] = {
        &NetConnServiceStub::OnGetNetCapabilities, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESSES_BY_NAME)] = {
        &NetConnServiceStub::OnGetAddressesByName, {Permission::INTERNET}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESS_BY_NAME)] = {
        &NetConnServiceStub::OnGetAddressByName, {Permission::INTERNET}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_IS_DEFAULT_NET_METERED)] = {
        &NetConnServiceStub::OnIsDefaultNetMetered, {Permission::GET_NETWORK_INFO}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_GLOBAL_HTTP_PROXY)] = {
        &NetConnServiceStub::OnGetGlobalHttpProxy, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_DEFAULT_HTTP_PROXY)] = {
        &NetConnServiceStub::OnGetDefaultHttpProxy, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_ID_BY_IDENTIFIER)] = {
        &NetConnServiceStub::OnGetNetIdByIdentifier, {}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_INTERFACE_CONFIGURATION)] = {
        &NetConnServiceStub::OnGetNetInterfaceConfiguration, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_SLOT_TYPE)] = {
        &NetConnServiceStub::OnRegisterSlotType, {Permission::CONNECTIVITY_INTERNAL}};
    memberFuncMap_[static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SLOT_TYPE)] = {
        &NetConnServiceStub::OnGetSlotType, {Permission::GET_NETWORK_INFO}};
}

NetConnServiceStub::~NetConnServiceStub() {}

std::string ToUtf8(std::u16string str16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(str16);
}

int32_t NetConnServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                            MessageOption &option)
{
    NETMGR_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = NetConnServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    NETMGR_LOG_D("myDescripter[%{public}s], remoteDescripter[%{public}s]", ToUtf8(myDescripter).c_str(),
                 ToUtf8(remoteDescripter).c_str());
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_E("descriptor checked fail");
        if (!reply.WriteInt32(NETMANAGER_ERR_DESCRIPTOR_MISMATCH)) {
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc == memberFuncMap_.end()) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto requestFunc = itFunc->second.first;
    if (requestFunc == nullptr) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    if (code == static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_INTERNET_PERMISSION)) {
        // get uid should be called in this function
        auto uid = IPCSkeleton::GetCallingUid();
        if (uid != UID_FOUNDATION && uid != UID_BROKER_SERVICE) {
            if (!reply.WriteInt32(NETMANAGER_ERR_PERMISSION_DENIED)) {
                return IPC_STUB_WRITE_PARCEL_ERR;
            }
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }

    int32_t ret = OnRequestCheck(code, itFunc->second.second);
    if (ret == NETMANAGER_SUCCESS) {
        ret =(this->*requestFunc)(data, reply);
        NETMGR_LOG_D("stub call end, code = [%{public}d]", code);
        return ret;
    }
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    NETMGR_LOG_D("stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetConnServiceStub::OnRequestCheck(uint32_t code, const std::set<std::string> &permissions)
{
    if (std::find(SYSTEM_CODE.begin(), SYSTEM_CODE.end(), code) != SYSTEM_CODE.end()) {
        if (!NetManagerPermission::IsSystemCaller()) {
            NETMGR_LOG_E("Non-system applications use system APIs.");
            return NETMANAGER_ERR_NOT_SYSTEM_CALL;
        }
    }

    if (std::find(PERMISSION_NEED_CACHE_CODES.begin(), PERMISSION_NEED_CACHE_CODES.end(), code) !=
        PERMISSION_NEED_CACHE_CODES.end()) {
        if (CheckPermissionWithCache(permissions)) {
            return NETMANAGER_SUCCESS;
        }
    } else {
        if (CheckPermission(permissions)) {
            return NETMANAGER_SUCCESS;
        }
    }
    return NETMANAGER_ERR_PERMISSION_DENIED;
}

bool NetConnServiceStub::CheckPermission(const std::set<std::string> &permissions)
{
    for (const auto &permission : permissions) {
        if (!NetManagerPermission::CheckPermission(permission)) {
            return false;
        }
    }
    return true;
}

bool NetConnServiceStub::CheckPermissionWithCache(const std::set<std::string> &permissions)
{
    for (const auto &permission : permissions) {
        if (!NetManagerPermission::CheckPermissionWithCache(permission)) {
            return false;
        }
    }
    return true;
}

int32_t NetConnServiceStub::OnSystemReady(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = SystemReady();
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return ret;
}

int32_t NetConnServiceStub::OnSetInternetPermission(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    uint8_t allow;
    if (!data.ReadUint8(allow)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = SetInternetPermission(uid, allow);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnRegisterNetSupplier(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub processing");
    NetBearType bearerType;
    std::string ident;
    std::set<NetCap> netCaps;

    uint32_t type = 0;
    if (!data.ReadUint32(type)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (type > static_cast<uint32_t>(NetBearType::BEARER_DEFAULT)) {
        return NETMANAGER_ERR_INTERNAL;
    }
    bearerType = static_cast<NetBearType>(type);

    if (!data.ReadString(ident)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    uint32_t size = 0;
    uint32_t value = 0;
    if (!data.ReadUint32(size)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    size = size > MAX_NET_CAP_NUM ? MAX_NET_CAP_NUM : size;
    for (uint32_t i = 0; i < size; ++i) {
        if (!data.ReadUint32(value)) {
            return NETMANAGER_ERR_READ_DATA_FAIL;
        }
        if (value < NET_CAPABILITY_END) {
            netCaps.insert(static_cast<NetCap>(value));
        }
    }

    uint32_t supplierId = 0;
    int32_t ret = RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        NETMGR_LOG_D("supplierId[%{public}d].", supplierId);
        if (!reply.WriteUint32(supplierId)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnUnregisterNetSupplier(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId;
    if (!data.ReadUint32(supplierId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = UnregisterNetSupplier(supplierId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnRegisterNetSupplierCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    uint32_t supplierId;
    data.ReadUint32(supplierId);
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetSupplierCallback> callback = iface_cast<INetSupplierCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = RegisterNetSupplierCallback(supplierId, callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnRegisterNetConnCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = RegisterNetConnCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnRegisterNetConnCallbackBySpecifier(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetSpecifier> netSpecifier = NetSpecifier::Unmarshalling(data);
    uint32_t timeoutMS = data.ReadUint32();
    int32_t result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = RegisterNetConnCallback(netSpecifier, callback, timeoutMS);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnRequestNetConnectionBySpecifier(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetSpecifier> netSpecifier = NetSpecifier::Unmarshalling(data);
    uint32_t timeoutMS = data.ReadUint32();
    int32_t result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = RequestNetConnection(netSpecifier, callback, timeoutMS);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUnregisterNetConnCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = UnregisterNetConnCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUpdateNetStateForTest(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("Test NetConnServiceStub::OnUpdateNetStateForTest(), begin");
    sptr<NetSpecifier> netSpecifier = NetSpecifier::Unmarshalling(data);

    int32_t netState;
    if (!data.ReadInt32(netState)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    NETMGR_LOG_D("Test NetConnServiceStub::OnUpdateNetStateForTest(), netState[%{public}d]", netState);
    int32_t result = UpdateNetStateForTest(netSpecifier, netState);
    NETMGR_LOG_D("Test NetConnServiceStub::OnUpdateNetStateForTest(), result[%{public}d]", result);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUpdateNetSupplierInfo(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("OnUpdateNetSupplierInfo in.");
    uint32_t supplierId;
    if (!data.ReadUint32(supplierId)) {
        NETMGR_LOG_D("fail to get supplier id.");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    NETMGR_LOG_D("OnUpdateNetSupplierInfo supplierId=[%{public}d].", supplierId);
    sptr<NetSupplierInfo> netSupplierInfo = NetSupplierInfo::Unmarshalling(data);
    int32_t ret = UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    if (!reply.WriteInt32(ret)) {
        NETMGR_LOG_D("fail to update net supplier info.");
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    NETMGR_LOG_D("OnUpdateNetSupplierInfo out.");

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnUpdateNetLinkInfo(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId;

    if (!data.ReadUint32(supplierId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    sptr<NetLinkInfo> netLinkInfo = NetLinkInfo::Unmarshalling(data);

    int32_t ret = UpdateNetLinkInfo(supplierId, netLinkInfo);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnRegisterNetDetectionCallback(MessageParcel &data, MessageParcel &reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_E("Execute ContainFileDescriptors failed");
    }
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetDetectionCallback> callback = iface_cast<INetDetectionCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = RegisterNetDetectionCallback(netId, callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUnRegisterNetDetectionCallback(MessageParcel &data, MessageParcel &reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_E("Execute ContainFileDescriptors failed");
    }
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetDetectionCallback> callback = iface_cast<INetDetectionCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        return result;
    }

    result = UnRegisterNetDetectionCallback(netId, callback);
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnNetDetection(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = NetDetection(netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnGetIfaceNames(MessageParcel &data, MessageParcel &reply)
{
    uint32_t netType = 0;
    if (!data.ReadUint32(netType)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (netType > static_cast<uint32_t>(NetBearType::BEARER_DEFAULT)) {
        return NETMANAGER_ERR_INTERNAL;
    }
    NetBearType bearerType = static_cast<NetBearType>(netType);
    std::list<std::string> ifaceNames;
    int32_t ret = GetIfaceNames(bearerType, ifaceNames);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(ifaceNames.size())) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        for (const auto &ifaceName : ifaceNames) {
            if (!reply.WriteString(ifaceName)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetIfaceNameByType(MessageParcel &data, MessageParcel &reply)
{
    uint32_t netType = 0;
    if (!data.ReadUint32(netType)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (netType > static_cast<uint32_t>(NetBearType::BEARER_DEFAULT)) {
        return NETMANAGER_ERR_INTERNAL;
    }
    NetBearType bearerType = static_cast<NetBearType>(netType);

    std::string ident;
    if (!data.ReadString(ident)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ifaceName;
    int32_t ret = GetIfaceNameByType(bearerType, ident, ifaceName);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteString(ifaceName)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetIfaceNameIdentMaps(MessageParcel &data, MessageParcel &reply)
{
    uint32_t netType = 0;
    if (!data.ReadUint32(netType)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (netType > static_cast<uint32_t>(NetBearType::BEARER_DEFAULT)) {
        return NETMANAGER_ERR_INTERNAL;
    }
    NetBearType bearerType = static_cast<NetBearType>(netType);
    std::unordered_map<std::string, std::string> ifaceNameIdentMaps;
    int32_t ret = GetIfaceNameIdentMaps(bearerType, ifaceNameIdentMaps);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(ifaceNameIdentMaps.size())) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        for (const auto &item: ifaceNameIdentMaps) {
            if (!reply.WriteString(item.first) || !reply.WriteString(item.second)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetDefaultNet(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("OnGetDefaultNet Begin...");
    int32_t netId;
    int32_t result = GetDefaultNet(netId);
    NETMGR_LOG_D("GetDefaultNet result is: [%{public}d]", result);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(netId)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnHasDefaultNet(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("OnHasDefaultNet Begin...");
    bool flag = false;
    int32_t result = HasDefaultNet(flag);
    NETMGR_LOG_D("HasDefaultNet result is: [%{public}d]", result);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteBool(flag)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnGetSpecificNet(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type;
    if (!data.ReadUint32(type)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    if (type > static_cast<uint32_t>(NetBearType::BEARER_DEFAULT)) {
        return NETMANAGER_ERR_INTERNAL;
    }

    NetBearType bearerType = static_cast<NetBearType>(type);

    NETMGR_LOG_D("stub execute GetSpecificNet");
    std::list<int32_t> netIdList;
    int32_t ret = GetSpecificNet(bearerType, netIdList);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        uint32_t size = static_cast<uint32_t>(netIdList.size());
        size = size > MAX_IFACE_NUM ? MAX_IFACE_NUM : size;
        if (!reply.WriteUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        uint32_t index = 0;
        for (auto p = netIdList.begin(); p != netIdList.end(); ++p) {
            if (++index > MAX_IFACE_NUM) {
                break;
            }
            if (!reply.WriteInt32(*p)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetAllNets(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub execute GetAllNets");
    std::list<int32_t> netIdList;
    int32_t ret = GetAllNets(netIdList);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        uint32_t size = static_cast<uint32_t>(netIdList.size());
        if (!reply.WriteUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }

        for (auto p = netIdList.begin(); p != netIdList.end(); ++p) {
            if (!reply.WriteInt32(*p)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetSpecificUidNet(MessageParcel &data, MessageParcel &reply)
{
    int32_t uid = 0;
    if (!data.ReadInt32(uid)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    NETMGR_LOG_D("stub execute GetSpecificUidNet");

    int32_t netId = 0;
    int32_t ret = GetSpecificUidNet(uid, netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteInt32(netId)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetConnectionProperties(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    NETMGR_LOG_D("stub execute GetConnectionProperties");
    NetLinkInfo info;
    int32_t ret = GetConnectionProperties(netId, info);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        sptr<NetLinkInfo> netLinkInfo_ptr = new (std::nothrow) NetLinkInfo(info);
        if (!NetLinkInfo::Marshalling(reply, netLinkInfo_ptr)) {
            NETMGR_LOG_E("proxy Marshalling failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetNetCapabilities(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    NETMGR_LOG_D("stub execute GetNetCapabilities");

    NetAllCapabilities netAllCap;
    int32_t ret = GetNetCapabilities(netId, netAllCap);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        if (!reply.WriteUint32(netAllCap.linkUpBandwidthKbps_) ||
            !reply.WriteUint32(netAllCap.linkDownBandwidthKbps_)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        uint32_t size = netAllCap.netCaps_.size();
        size = size > MAX_NET_CAP_NUM ? MAX_NET_CAP_NUM : size;
        if (!reply.WriteUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        uint32_t index = 0;
        for (auto netCap : netAllCap.netCaps_) {
            if (++index > MAX_NET_CAP_NUM) {
                break;
            }
            if (!reply.WriteUint32(static_cast<uint32_t>(netCap))) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }

        size = netAllCap.bearerTypes_.size();
        size = size > MAX_NET_CAP_NUM ? MAX_NET_CAP_NUM : size;
        if (!reply.WriteUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        index = 0;
        for (auto bearerType : netAllCap.bearerTypes_) {
            if (++index > MAX_NET_CAP_NUM) {
                break;
            }
            if (!reply.WriteUint32(static_cast<uint32_t>(bearerType))) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetAddressesByName(MessageParcel &data, MessageParcel &reply)
{
    std::string host;
    if (!data.ReadString(host)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    NETMGR_LOG_D("stub execute GetAddressesByName");
    std::vector<INetAddr> addrList;
    int32_t ret = GetAddressesByName(host, netId, addrList);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        uint32_t size = static_cast<uint32_t>(addrList.size());
        size = size > MAX_IFACE_NUM ? MAX_IFACE_NUM : size;
        if (!reply.WriteUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        uint32_t index = 0;
        for (auto p = addrList.begin(); p != addrList.end(); ++p) {
            if (++index > MAX_IFACE_NUM) {
                break;
            }
            sptr<INetAddr> netaddr_ptr = (std::make_unique<INetAddr>(*p)).release();
            if (!INetAddr::Marshalling(reply, netaddr_ptr)) {
                NETMGR_LOG_E("proxy Marshalling failed");
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetAddressByName(MessageParcel &data, MessageParcel &reply)
{
    std::string host;
    if (!data.ReadString(host)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    NETMGR_LOG_D("stub execute GetAddressByName");
    INetAddr addr;
    int32_t ret = GetAddressByName(host, netId, addr);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (ret == NETMANAGER_SUCCESS) {
        sptr<INetAddr> netaddr_ptr = (std::make_unique<INetAddr>(addr)).release();
        if (!INetAddr::Marshalling(reply, netaddr_ptr)) {
            NETMGR_LOG_E("proxy Marshalling failed");
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnBindSocket(MessageParcel &data, MessageParcel &reply)
{
    int32_t socketFd;
    if (!data.ReadInt32(socketFd)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t netId;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    NETMGR_LOG_D("stub execute BindSocket");

    int32_t ret = BindSocket(socketFd, netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return ret;
}

int32_t NetConnServiceStub::OnSetAirplaneMode(MessageParcel &data, MessageParcel &reply)
{
    bool state = false;
    if (!data.ReadBool(state)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t ret = SetAirplaneMode(state);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return ret;
}

int32_t NetConnServiceStub::OnIsDefaultNetMetered(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub execute IsDefaultNetMetered");
    bool flag = false;
    int32_t result = IsDefaultNetMetered(flag);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (result == NETMANAGER_SUCCESS) {
        if (!reply.WriteBool(flag)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnSetGlobalHttpProxy(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub execute SetGlobalHttpProxy");

    HttpProxy httpProxy;
    if (!HttpProxy::Unmarshalling(data, httpProxy)) {
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = SetGlobalHttpProxy(httpProxy);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetGlobalHttpProxy(MessageParcel &data, MessageParcel &reply)
{
    HttpProxy httpProxy;
    int32_t result = GetGlobalHttpProxy(httpProxy);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (result != NETMANAGER_SUCCESS) {
        return result;
    }

    if (!httpProxy.Marshalling(reply)) {
        return ERR_FLATTEN_OBJECT;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnGetDefaultHttpProxy(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub execute OnGetDefaultHttpProxy");
    int32_t bindNetId = 0;
    if (!data.ReadInt32(bindNetId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    HttpProxy httpProxy;
    int32_t result = GetDefaultHttpProxy(bindNetId, httpProxy);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (result != NETMANAGER_SUCCESS) {
        return result;
    }

    if (!httpProxy.Marshalling(reply)) {
        return ERR_FLATTEN_OBJECT;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnGetNetIdByIdentifier(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOG_D("stub execute OnGetNetIdByIdentifier");
    std::string ident;
    if (!data.ReadString(ident)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::list<int32_t> netIdList;
    int32_t ret = GetNetIdByIdentifier(ident, netIdList);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (ret == NETMANAGER_SUCCESS) {
        uint32_t size = static_cast<uint32_t>(netIdList.size());
        if (!reply.WriteUint32(size)) {
            return NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        for (auto p = netIdList.begin(); p != netIdList.end(); ++p) {
            if (!reply.WriteInt32(*p)) {
                return NETMANAGER_ERR_WRITE_REPLY_FAIL;
            }
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnSetAppNet(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int ret = SetAppNet(netId);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return ret;
}

int32_t NetConnServiceStub::OnRegisterNetInterfaceCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("Callback ptr is nullptr.");
        ret = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(ret);
        return ret;
    }

    sptr<INetInterfaceStateCallback> callback = iface_cast<INetInterfaceStateCallback>(remote);
    ret = RegisterNetInterfaceCallback(callback);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return ret;
}

int32_t NetConnServiceStub::OnGetNetInterfaceConfiguration(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    NetInterfaceConfiguration config;
    int32_t ret = GetNetInterfaceConfiguration(iface, config);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    if (ret == NETMANAGER_SUCCESS) {
        if (!config.Marshalling(reply)) {
            return ERR_FLATTEN_OBJECT;
        }
    }
    return ret;
}

int32_t NetConnServiceStub::OnAddNetworkRoute(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string destination = "";
    if (!data.ReadString(destination)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string nextHop = "";
    if (!data.ReadString(nextHop)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = AddNetworkRoute(netId, ifName, destination, nextHop);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}


int32_t NetConnServiceStub::OnRemoveNetworkRoute(MessageParcel &data, MessageParcel &reply)
{
    int32_t netId = 0;
    if (!data.ReadInt32(netId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string destination = "";
    if (!data.ReadString(destination)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string nextHop = "";
    if (!data.ReadString(nextHop)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = RemoveNetworkRoute(netId, ifName, destination, nextHop);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnAddInterfaceAddress(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ipAddr = "";
    if (!data.ReadString(ipAddr)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t prefixLength = 0;
    if (!data.ReadInt32(prefixLength)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = AddInterfaceAddress(ifName, ipAddr, prefixLength);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnDelInterfaceAddress(MessageParcel &data, MessageParcel &reply)
{
    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ipAddr = "";
    if (!data.ReadString(ipAddr)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t prefixLength = 0;
    if (!data.ReadInt32(prefixLength)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = DelInterfaceAddress(ifName, ipAddr, prefixLength);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnAddStaticArp(MessageParcel &data, MessageParcel &reply)
{
    std::string ipAddr = "";
    if (!data.ReadString(ipAddr)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string macAddr = "";
    if (!data.ReadString(macAddr)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = AddStaticArp(ipAddr, macAddr, ifName);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnDelStaticArp(MessageParcel &data, MessageParcel &reply)
{
    std::string ipAddr = "";
    if (!data.ReadString(ipAddr)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string macAddr = "";
    if (!data.ReadString(macAddr)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    std::string ifName = "";
    if (!data.ReadString(ifName)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = DelStaticArp(ipAddr, macAddr, ifName);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnRegisterSlotType(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId = 0;
    if (!data.ReadUint32(supplierId)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t type = 0;
    if (!data.ReadInt32(type)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t ret = RegisterSlotType(supplierId, type);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnGetSlotType(MessageParcel &data, MessageParcel &reply)
{
    std::string type = "";
    int32_t ret = GetSlotType(type);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (!reply.WriteString(type)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnFactoryResetNetwork(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = FactoryResetNetwork();
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnRegisterNetFactoryResetCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("remote ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetFactoryResetCallback> callback = iface_cast<INetFactoryResetCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        NETMGR_LOG_E("Callback ptr is nullptr.");
        return result;
    }

    result = RegisterNetFactoryResetCallback(callback);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}
 
int32_t NetConnServiceStub::OnIsPreferCellularUrl(MessageParcel &data, MessageParcel &reply)
{
    std::string url;
    if (!data.ReadString(url)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    bool preferCellular = false;
    int32_t ret = IsPreferCellularUrl(url, preferCellular);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    if (!reply.WriteBool(preferCellular)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
 
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnUpdateSupplierScore(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    bool isBetter;
    if (!data.ReadUint32(type)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (!data.ReadBool(isBetter)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (type > static_cast<uint32_t>(NetBearType::BEARER_DEFAULT)) {
        return NETMANAGER_ERR_INTERNAL;
    }
    NetBearType bearerType = static_cast<NetBearType>(type);
    int32_t ret = UpdateSupplierScore(bearerType, isBetter);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnRegisterPreAirplaneCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("remote ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<IPreAirplaneCallback> callback = iface_cast<IPreAirplaneCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        NETMGR_LOG_E("Callback ptr is nullptr.");
        return result;
    }

    result = RegisterPreAirplaneCallback(callback);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnServiceStub::OnUnregisterPreAirplaneCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NETMANAGER_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOG_E("remote ptr is nullptr.");
        result = NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
        reply.WriteInt32(result);
        return result;
    }

    sptr<IPreAirplaneCallback> callback = iface_cast<IPreAirplaneCallback>(remote);
    if (callback == nullptr) {
        result = NETMANAGER_ERR_LOCAL_PTR_NULL;
        reply.WriteInt32(result);
        NETMGR_LOG_E("Callback ptr is nullptr.");
        return result;
    }

    result = UnregisterPreAirplaneCallback(callback);
    if (!reply.WriteInt32(result)) {
        return NETMANAGER_ERR_WRITE_REPLY_FAIL;
    }
    
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
