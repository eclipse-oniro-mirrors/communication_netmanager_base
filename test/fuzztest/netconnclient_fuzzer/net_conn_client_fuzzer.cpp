/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <securec.h>
#include <thread>

#include "common_net_conn_callback_test.h"
#include "i_net_supplier_callback.h"
#include "iservice_registry.h"
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_test_security.h"
#include "system_ability_definition.h"
#define private public
#include "net_conn_client.h"
#include "net_conn_service.h"
#include "net_conn_service_stub.h"
#include "net_interface_callback_stub.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_NET_TYPE_VALUE = 7;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
} // namespace

template <class T> T NetConnGetData()
{
    T object{};
    size_t netConnSize = sizeof(object);
    if (g_baseFuzzData == nullptr || netConnSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, netConnSize, g_baseFuzzData + g_baseFuzzPos, netConnSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += netConnSize;
    return object;
}

std::string NetConnGetString(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetConnGetData<char>();
    }
    std::string str(cstr);
    return str;
}

SecureData GetSecureDataFromData(int8_t strlen)
{
    SecureData secureData;
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetConnGetData<char>();
    }
    secureData.append(cstr, strlen-1);
    return secureData;
}

class INetDetectionCallbackTest : public IRemoteStub<INetDetectionCallback> {
public:
    virtual int32_t OnNetDetectionResultChanged(NetDetectionResultCode detectionResult, const std::string &urlRedirect)
    {
        return 0;
    }
};

class NetInterfaceStateCallbackTest : public NetInterfaceStateCallbackStub {};

static bool g_isInited = false;
void Init()
{
    if (!g_isInited) {
        if (!DelayedSingleton<NetConnService>::GetInstance()->Init()) {
            g_isInited = false;
        } else {
            g_isInited = true;
        }
    }
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        NETMGR_LOG_D("Net conn client fuzz test g_isInited is false.");
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    int32_t ret = DelayedSingleton<NetConnService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
    return ret;
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetConnServiceStub::GetDescriptor())) {
        NETMGR_LOG_D("Write token failed.");
        return false;
    }
    return true;
}

bool IsConnClientDataAndSizeValid(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if ((data == nullptr) || (size == 0)) {
        NETMGR_LOG_D("IsConnClientDataAndSizeValid data is nullptr or size is zero.");
        return false;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    if (!WriteInterfaceToken(dataParcel)) {
        return false;
    }
    return true;
}

void SetAppHttpProxyCallback(const HttpProxy &httpProxy)
{
    return;
}

void SystemReadyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SYSTEM_READY), dataParcel);
}

void RegisterNetSupplierFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint32_t bearerType = NetConnGetData<uint32_t>() % CREATE_NET_TYPE_VALUE;
    dataParcel.WriteUint32(bearerType);

    std::string ident = NetConnGetString(STR_LEN);
    dataParcel.WriteString(ident);

    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    uint32_t capsSize = static_cast<uint32_t>(netCaps.size());
    dataParcel.WriteUint32(capsSize);
    for (auto netCap : netCaps) {
        dataParcel.WriteUint32(static_cast<uint32_t>(netCap));
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REG_NET_SUPPLIER), dataParcel);
}

void UnregisterNetSupplierFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = NetConnGetData<uint32_t>();
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREG_NETWORK), dataParcel);
}

void HasDefaultNetFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_HASDEFAULTNET), dataParcel);
}

void GetAllNetsFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ALL_NETS), dataParcel);
}

void BindSocketFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    int32_t socketFd = NetConnGetData<int32_t>();
    int32_t netId = NetConnGetData<int32_t>();
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(socketFd);
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_BIND_SOCKET), dataParcel);
}

void SetAirplaneModeFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    bool state = NetConnGetData<bool>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteBool(state);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_AIRPLANE_MODE), dataParcel);

    MessageParcel dataParcelNoState;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoState)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_AIRPLANE_MODE), dataParcelNoState);
}

void UpdateNetSupplierInfoFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = NetConnGetData<uint32_t>();
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("UpdateNetSupplierInfoFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcel.WriteUint32(supplierId);
    netSupplierInfo->Marshalling(dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_SUPPLIER_INFO), dataParcel);

    MessageParcel dataParcelNoSupplierId;
    netSupplierInfo->Marshalling(dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_SUPPLIER_INFO), dataParcel);
}

void GetAddressByNameFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    std::string host = NetConnGetString(STR_LEN);
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("GetAddressByNameFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcel.WriteString(host);
    dataParcel.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESS_BY_NAME), dataParcel);

    MessageParcel dataParcelNoHost;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoHost)) {
        NETMGR_LOG_D("GetAddressByNameFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoHost.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESS_BY_NAME), dataParcelNoHost);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        NETMGR_LOG_D("GetAddressByNameFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoNetId.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESS_BY_NAME), dataParcelNoNetId);
}

void GetAddressesByNameFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    std::string host = NetConnGetString(STR_LEN);
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteString(host);
    dataParcel.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESSES_BY_NAME), dataParcel);

    MessageParcel dataParcelNoHost;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoHost)) {
        return;
    }
    dataParcelNoHost.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESSES_BY_NAME), dataParcelNoHost);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }
    dataParcelNoNetId.WriteString(host);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_ADDRESSES_BY_NAME), dataParcelNoNetId);
}

void UpdateNetLinkInfoFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t supplierId = NetConnGetData<uint32_t>();
    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    if (netLinkInfo == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    netLinkInfo->Marshalling(dataParcel);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_LINK_INFO), dataParcel);

    MessageParcel dataParcelNoSupplierId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoSupplierId)) {
        return;
    }
    netLinkInfo->Marshalling(dataParcelNoSupplierId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_NET_LINK_INFO), dataParcelNoSupplierId);
}

void RegisterNetSupplierCallbackFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    uint32_t supplierId = NetConnGetData<uint32_t>();
    sptr<NetSupplierCallbackStubTestCb> callback = new (std::nothrow) NetSupplierCallbackStubTestCb();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK), dataParcel);

    MessageParcel dataParcelNoSupplierId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcelNoSupplierId.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK),
                    dataParcelNoSupplierId);
    
    MessageParcel dataParcelNoRemoteObject;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoRemoteObject)) {
        return;
    }
    dataParcelNoRemoteObject.WriteUint32(supplierId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK),
                    dataParcelNoRemoteObject);
}

void RegisterNetConnCallbackBySpecifierFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    if (netSpecifier == nullptr || callback == nullptr) {
        return;
    }
    uint32_t timeoutMS = NetConnGetData<uint32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteUint32(timeoutMS);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER),
                    dataParcel);

    MessageParcel dataParcelNoNetSpecifier;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetSpecifier)) {
        return;
    }
    netSpecifier->Marshalling(dataParcelNoNetSpecifier);
    dataParcelNoNetSpecifier.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER),
                    dataParcelNoNetSpecifier);
    
    MessageParcel dataParcelNoRemoteObject;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoRemoteObject)) {
        return;
    }
    netSpecifier->Marshalling(dataParcelNoRemoteObject);
    dataParcelNoRemoteObject.WriteUint32(timeoutMS);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER),
                    dataParcelNoRemoteObject);
}

void RegisterNetConnCallbackFuzzTest(const uint8_t *data, size_t size)
{
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK), dataParcel);

    MessageParcel dataParcelNoRemoteObject;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoRemoteObject)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_CONN_CALLBACK),
                    dataParcelNoRemoteObject);
}

void UnregisterNetConnCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    NetManagerBaseAccessToken token;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_CONN_CALLBACK), dataParcel);
    
    MessageParcel dataParcelNoRemoteObject;
    if (!WriteInterfaceToken(dataParcelNoRemoteObject)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_CONN_CALLBACK),
                    dataParcelNoRemoteObject);
}

void GetDefaultNetFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GETDEFAULTNETWORK), dataParcel);
}

void GetConnectionPropertiesFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_CONNECTION_PROPERTIES), dataParcel);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }
    dataParcelNoNetId.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_CONNECTION_PROPERTIES), dataParcelNoNetId);
}

void GetNetCapabilitiesFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_CAPABILITIES), dataParcel);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_CAPABILITIES), dataParcelNoNetId);
}

void NetDetectionFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken tokenInternetInfo;
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_NET_DETECTION), dataParcel);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }
    dataParcelNoNetId.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_NET_DETECTION), dataParcelNoNetId);
}

void IsDefaultNetMeteredFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_IS_DEFAULT_NET_METERED), dataParcel);
}


void SetAppHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    HttpProxy httpProxy = {NetConnGetString(STR_LEN), 0, {}};
    DelayedSingleton<NetConnClient>::GetInstance()->SetAppHttpProxy(httpProxy);
}

void RegisterAppHttpProxyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t callbackId = 0;
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterAppHttpProxyCallback(SetAppHttpProxyCallback, callbackId);
}

void UnregisterAppHttpProxyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    int32_t callbackId = NetConnGetData<int32_t>();
    DelayedSingleton<NetConnClient>::GetInstance()->UnregisterAppHttpProxyCallback(callbackId);
}

void SetGlobalHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    HttpProxy httpProxy = {NetConnGetString(STR_LEN), 0, {}};
    httpProxy.SetUserName(GetSecureDataFromData(STR_LEN));
    httpProxy.SetPassword(GetSecureDataFromData(STR_LEN));
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    httpProxy.Marshalling(dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_GLOBAL_HTTP_PROXY), dataParcel);
}

void GetGlobalHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_GLOBAL_HTTP_PROXY), dataParcel);
}

void GetDefaultHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_DEFAULT_HTTP_PROXY), dataParcel);
}

void GetNetIdByIdentifierFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    std::string ident = NetConnGetString(STR_LEN);

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteString(ident);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_ID_BY_IDENTIFIER), dataParcel);

    MessageParcel dataParcelNoIdent;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIdent)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_NET_ID_BY_IDENTIFIER), dataParcelNoIdent);
}

void RegisterNetInterfaceCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    NetManagerBaseAccessToken token;
    sptr<INetInterfaceStateCallback> callback = new (std::nothrow) NetInterfaceStateCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_INTERFACE_CALLBACK), dataParcel);

    MessageParcel dataParcelNoRemoteObject;
    if (!WriteInterfaceToken(dataParcelNoRemoteObject)) {
        return;
    }
    dataParcelNoRemoteObject.WriteRemoteObject(callback->AsObject().GetRefPtr());
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_INTERFACE_CALLBACK),
                    dataParcelNoRemoteObject);
}

void GetNetInterfaceConfigurationFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    std::string iface = NetConnGetString(STR_LEN);

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteString(iface);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_INTERFACE_CONFIGURATION), dataParcel);
    
    MessageParcel dataParcelNoIface;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIface)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_INTERFACE_CONFIGURATION), dataParcelNoIface);
}

void SetInternetPermissionFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t uid = NetConnGetData<uint32_t>();
    uint8_t allow = NetConnGetData<uint8_t>();

    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteUint32(uid);
    dataParcel.WriteUint8(allow);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_INTERNET_PERMISSION), dataParcel);

    MessageParcel dataParcelNoUid;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoUid)) {
        return;
    }

    dataParcelNoUid.WriteUint8(allow);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_INTERNET_PERMISSION), dataParcelNoUid);
    
    MessageParcel dataParcelNoAllow;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoAllow)) {
        return;
    }

    dataParcel.WriteUint32(uid);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_INTERNET_PERMISSION), dataParcelNoAllow);
}

void UpdateNetStateForTestFuzzTest(const uint8_t *data, size_t size)
{
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    if (netSpecifier == nullptr) {
        return;
    }
    auto netState = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("UpdateNetSupplierInfoFuzzTest write token failed or invalid parameter.");
        return;
    }

    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteInt32(netState);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UPDATE_NET_STATE_FOR_TEST), dataParcel);

    MessageParcel dataParcelNoNetState;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetState)) {
        NETMGR_LOG_D("UpdateNetSupplierInfoFuzzTest write token failed or invalid parameter.");
        return;
    }

    netSpecifier->Marshalling(dataParcelNoNetState);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UPDATE_NET_STATE_FOR_TEST), dataParcelNoNetState);
}

void GetIfaceNamesFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t bearerType = NetConnGetData<uint32_t>() % CREATE_NET_TYPE_VALUE;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("GetIfaceNamesFuzzTest write token failed or invalid parameter.");
        return;
    }

    dataParcel.WriteUint32(bearerType);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACE_NAMES), dataParcel);

    MessageParcel dataParcelNoBearerType;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoBearerType)) {
        NETMGR_LOG_D("GetIfaceNamesFuzzTest write token failed or invalid parameter.");
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACE_NAMES), dataParcelNoBearerType);
}

void GetIfaceNameByTypeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t bearerType = NetConnGetData<uint32_t>() % CREATE_NET_TYPE_VALUE;
    std::string ident = NetConnGetString(STR_LEN);

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("GetIfaceNameByTypeFuzzTest write token failed or invalid parameter.");
        return;
    }

    dataParcel.WriteUint32(bearerType);
    dataParcel.WriteString(ident);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACENAME_BY_TYPE), dataParcel);

    MessageParcel dataParcelNoBearerType;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoBearerType)) {
        NETMGR_LOG_D("GetIfaceNameByTypeFuzzTest write token failed or invalid parameter.");
        return;
    }

    dataParcelNoBearerType.WriteString(ident);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACENAME_BY_TYPE), dataParcelNoBearerType);

    MessageParcel dataParcelNoIdent;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIdent)) {
        NETMGR_LOG_D("GetIfaceNameByTypeFuzzTest write token failed or invalid parameter.");
        return;
    }

    dataParcel.WriteUint32(bearerType);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_IFACENAME_BY_TYPE), dataParcelNoIdent);
}

void RegisterNetDetectionCallbackFuzzTest(const uint8_t *data, size_t size)
{
    int32_t netId = NetConnGetData<int32_t>();
    sptr<INetDetectionCallbackTest> callback = new (std::nothrow) INetDetectionCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteInt32(netId);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK),
                    dataParcel);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }

    dataParcelNoNetId.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK),
                    dataParcelNoNetId);

    MessageParcel dataParcelNoRemoteObject;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoRemoteObject)) {
        return;
    }
    dataParcelNoRemoteObject.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK),
                    dataParcelNoRemoteObject);
}

void UnRegisterNetDetectionCallbackFuzzTest(const uint8_t *data, size_t size)
{
    int32_t netId = NetConnGetData<int32_t>();
    sptr<INetDetectionCallbackTest> callback = new (std::nothrow) INetDetectionCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteInt32(netId);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK),
                    dataParcel);
    
    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }

    dataParcelNoNetId.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK),
                    dataParcelNoNetId);

    MessageParcel dataParcelNoRemoteObject;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcelNoRemoteObject.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK),
                    dataParcelNoRemoteObject);
}

void GetSpecificNetFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t bearerType = NetConnGetData<uint32_t>() % CREATE_NET_TYPE_VALUE;

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteUint32(bearerType);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_NET), dataParcel);
    
    MessageParcel dataParcelNoBearerType;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoBearerType)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_NET), dataParcelNoBearerType);
}

void OnSetAppNetFuzzTest(const uint8_t *data, size_t size)
{
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_APP_NET), dataParcel);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_SET_APP_NET), dataParcelNoNetId);
}

void GetSpecificUidNetFuzzTest(const uint8_t *data, size_t size)
{
    int32_t uid = NetConnGetData<int32_t>();
    int32_t netId = NetConnGetData<int32_t>();

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteInt32(uid);
    dataParcel.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_UID_NET), dataParcel);

    MessageParcel dataParcelNoUid;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoUid)) {
        return;
    }

    dataParcelNoUid.WriteInt32(netId);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_UID_NET), dataParcelNoUid);

    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        return;
    }
    dataParcel.WriteInt32(uid);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_GET_SPECIFIC_UID_NET), dataParcelNoNetId);
}

void AddNetworkRouteFuzzTest(const uint8_t *data, size_t size)
{
    int32_t netId = NetConnGetData<int32_t>();
    std::string ifName = NetConnGetString(STR_LEN);
    std::string destination = NetConnGetString(STR_LEN);
    std::string nextHop = NetConnGetString(STR_LEN);
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("AddNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(destination);
    dataParcel.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ROUTE), dataParcel);
    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        NETMGR_LOG_D("AddNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoNetId.WriteString(ifName);
    dataParcelNoNetId.WriteString(destination);
    dataParcelNoNetId.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ROUTE), dataParcelNoNetId);
    MessageParcel dataParcelNoIfName;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIfName)) {
        NETMGR_LOG_D("AddNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoIfName.WriteInt32(netId);
    dataParcelNoIfName.WriteString(destination);
    dataParcelNoIfName.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ROUTE), dataParcelNoIfName);
    MessageParcel dataParcelNoDest;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoDest)) {
        NETMGR_LOG_D("AddNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoDest.WriteInt32(netId);
    dataParcelNoDest.WriteString(ifName);
    dataParcelNoDest.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ROUTE), dataParcelNoDest);
}

void RemoveNetworkRouteFuzzTest(const uint8_t *data, size_t size)
{
    int32_t netId = NetConnGetData<int32_t>();
    std::string ifName = NetConnGetString(STR_LEN);
    std::string destination = NetConnGetString(STR_LEN);
    std::string nextHop = NetConnGetString(STR_LEN);
    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("RemoveNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(destination);
    dataParcel.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ROUTE), dataParcel);
    MessageParcel dataParcelNoNetId;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoNetId)) {
        NETMGR_LOG_D("RemoveNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoNetId.WriteString(ifName);
    dataParcelNoNetId.WriteString(destination);
    dataParcelNoNetId.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ROUTE), dataParcelNoNetId);
    MessageParcel dataParcelNoIfName;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIfName)) {
        NETMGR_LOG_D("RemoveNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoIfName.WriteInt32(netId);
    dataParcelNoIfName.WriteString(destination);
    dataParcelNoIfName.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ROUTE), dataParcelNoIfName);
    MessageParcel dataParcelNoDest;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoDest)) {
        NETMGR_LOG_D("RemoveNetworkRouteFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoDest.WriteInt32(netId);
    dataParcelNoDest.WriteString(ifName);
    dataParcelNoDest.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ROUTE), dataParcelNoDest);
}

void AddInterfaceAddressFuzzTest(const uint8_t *data, size_t size)
{
    int32_t prefixLength = NetConnGetData<int32_t>();
    std::string ifName = NetConnGetString(STR_LEN);
    std::string ipAddr = NetConnGetString(STR_LEN);

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("AddInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS), dataParcel);

    MessageParcel dataParcelNoIfName;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIfName)) {
        NETMGR_LOG_D("AddInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoIfName.WriteString(ipAddr);
    dataParcelNoIfName.WriteInt32(prefixLength);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS), dataParcelNoIfName);

    MessageParcel dataParcelNoIpAddr;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIpAddr)) {
        NETMGR_LOG_D("AddInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoIpAddr.WriteString(ifName);
    dataParcelNoIpAddr.WriteInt32(prefixLength);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS), dataParcelNoIpAddr);

    MessageParcel dataParcelNoPrefix;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoPrefix)) {
        NETMGR_LOG_D("AddInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoPrefix.WriteString(ifName);
    dataParcelNoPrefix.WriteString(ipAddr);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_NET_ADDRESS), dataParcelNoPrefix);
}

void DelInterfaceAddressFuzzTest(const uint8_t *data, size_t size)
{
    int32_t prefixLength = NetConnGetData<int32_t>();
    std::string ifName = NetConnGetString(STR_LEN);
    std::string ipAddr = NetConnGetString(STR_LEN);

    MessageParcel dataParcel;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        NETMGR_LOG_D("DelInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ADDRESS), dataParcel);

    MessageParcel dataParcelNoIfName;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIfName)) {
        NETMGR_LOG_D("DelInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoIfName.WriteString(ipAddr);
    dataParcelNoIfName.WriteInt32(prefixLength);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ADDRESS), dataParcelNoIfName);

    MessageParcel dataParcelNoIpAddr;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoIpAddr)) {
        NETMGR_LOG_D("DelInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoIpAddr.WriteString(ifName);
    dataParcelNoIpAddr.WriteInt32(prefixLength);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ADDRESS), dataParcelNoIpAddr);

    MessageParcel dataParcelNoPrefix;
    if (!IsConnClientDataAndSizeValid(data, size, dataParcelNoPrefix)) {
        NETMGR_LOG_D("DelInterfaceAddressFuzzTest write token failed or invalid parameter.");
        return;
    }
    dataParcelNoPrefix.WriteString(ifName);
    dataParcelNoPrefix.WriteString(ipAddr);

    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_REMOVE_NET_ADDRESS), dataParcelNoPrefix);
}

void StaticArpProcess(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ipAddr = NetConnGetString(STR_LEN);
    std::string macAddr = NetConnGetString(STR_LEN);
    std::string ifName = NetConnGetString(STR_LEN);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteString(macAddr);
    dataParcel.WriteString(ifName);
}

void StaticArpProcessNoIpAddr(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string macAddr = NetConnGetString(STR_LEN);
    std::string ifName = NetConnGetString(STR_LEN);
    dataParcel.WriteString(macAddr);
    dataParcel.WriteString(ifName);
}

void StaticArpProcessNoMacAddr(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ipAddr = NetConnGetString(STR_LEN);
    std::string ifName = NetConnGetString(STR_LEN);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteString(ifName);
}

void StaticArpProcessNoIfName(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if (!IsConnClientDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ipAddr = NetConnGetString(STR_LEN);
    std::string macAddr = NetConnGetString(STR_LEN);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteString(macAddr);
}

void AddStaticArpFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    StaticArpProcess(data, size, dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_STATIC_ARP), dataParcel);

    MessageParcel dataParcelNoIpAddr;
    StaticArpProcessNoIpAddr(data, size, dataParcelNoIpAddr);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_STATIC_ARP), dataParcelNoIpAddr);

    MessageParcel dataParcelNoMacAddr;
    StaticArpProcessNoMacAddr(data, size, dataParcelNoMacAddr);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_STATIC_ARP), dataParcelNoMacAddr);

    MessageParcel dataParcelNoIfName;
    StaticArpProcessNoIfName(data, size, dataParcelNoIfName);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_ADD_STATIC_ARP), dataParcelNoIfName);
}

void DelStaticArpFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    StaticArpProcess(data, size, dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_DEL_STATIC_ARP), dataParcel);

    MessageParcel dataParcelNoIpAddr;
    StaticArpProcessNoIpAddr(data, size, dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_DEL_STATIC_ARP), dataParcelNoIpAddr);

    MessageParcel dataParcelNoMacAddr;
    StaticArpProcessNoMacAddr(data, size, dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_DEL_STATIC_ARP), dataParcelNoMacAddr);

    MessageParcel dataParcelNoIfName;
    StaticArpProcessNoIfName(data, size, dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(ConnInterfaceCode::CMD_NM_DEL_STATIC_ARP), dataParcelNoIfName);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::SystemReadyFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetSupplierFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetSupplierFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetSupplierCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateNetSupplierInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateNetLinkInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetConnCallbackBySpecifierFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetConnCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetConnCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetDefaultNetFuzzTest(data, size);
    OHOS::NetManagerStandard::HasDefaultNetFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllNetsFuzzTest(data, size);
    OHOS::NetManagerStandard::GetConnectionPropertiesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetCapabilitiesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAddressesByNameFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAddressByNameFuzzTest(data, size);
    OHOS::NetManagerStandard::BindSocketFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDetectionFuzzTest(data, size);
    OHOS::NetManagerStandard::SetAirplaneModeFuzzTest(data, size);
    OHOS::NetManagerStandard::IsDefaultNetMeteredFuzzTest(data, size);
    OHOS::NetManagerStandard::SetGlobalHttpProxyFuzzTest(data, size);
    OHOS::NetManagerStandard::GetGlobalHttpProxyFuzzTest(data, size);
    OHOS::NetManagerStandard::GetDefaultHttpProxyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetAppHttpProxyFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterAppHttpProxyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterAppHttpProxyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetIdByIdentifierFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetInterfaceCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetInterfaceConfigurationFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInternetPermissionFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateNetStateForTestFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceNamesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceNameByTypeFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetDetectionCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnRegisterNetDetectionCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetSpecificNetFuzzTest(data, size);
    OHOS::NetManagerStandard::GetSpecificUidNetFuzzTest(data, size);
    OHOS::NetManagerStandard::OnSetAppNetFuzzTest(data, size);
    OHOS::NetManagerStandard::AddNetworkRouteFuzzTest(data, size);
    OHOS::NetManagerStandard::RemoveNetworkRouteFuzzTest(data, size);
    OHOS::NetManagerStandard::AddInterfaceAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::DelInterfaceAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::AddStaticArpFuzzTest(data, size);
    OHOS::NetManagerStandard::DelStaticArpFuzzTest(data, size);
    return 0;
}
