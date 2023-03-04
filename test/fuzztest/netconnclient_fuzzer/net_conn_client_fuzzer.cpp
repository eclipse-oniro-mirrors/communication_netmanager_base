/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <thread>

#include <securec.h>

#include "accesstoken_kit.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "i_net_supplier_callback.h"
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_supplier_callback_stub.h"
#define private public
#include "net_conn_service.h"
#include "net_conn_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_NET_TYPE_VALUE = 7;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;

using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {.userID = 1,
                               .bundleName = "net_conn_client_fuzzer",
                               .instIndex = 0,
                               .appIDDesc = "test"};

PermissionDef testPermDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "net_conn_client_fuzzer",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect maneger network info",
    .descriptionId = 1,
};

PermissionDef testInternetPermDef = {.permissionName = "ohos.permission.INTERNET",
                                     .bundleName = "net_conn_client_fuzzer",
                                     .grantMode = 1,
                                     .availableLevel = APL_SYSTEM_BASIC,
                                     .label = "label",
                                     .labelId = 1,
                                     .description = "Test net connect maneger internet",
                                     .descriptionId = 1};

PermissionStateFull testState = {.permissionName = "ohos.permission.GET_NETWORK_INFO",
                                 .isGeneral = true,
                                 .resDeviceID = {"local"},
                                 .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                 .grantFlags = {2}};

PermissionStateFull testInternetState = {.permissionName = "ohos.permission.INTERNET",
                                         .isGeneral = true,
                                         .resDeviceID = {"local"},
                                         .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                         .grantFlags = {2}};

HapPolicyParams testPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                   .domain = "test.domain",
                                   .permList = {testPermDef},
                                   .permStateList = {testState}};

HapPolicyParams testInternetPolicyPrams = {.apl = APL_SYSTEM_BASIC,
                                           .domain = "test.domain",
                                           .permList = {testPermDef, testInternetPermDef},
                                           .permStateList = {testState, testInternetState}};
} // namespace

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

std::string GetStringFromData(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetData<char>();
    }
    std::string str(cstr);
    return str;
}

class AccessToken {
public:
    AccessToken() : currentID_(GetSelfTokenID())
    {
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_;
    AccessTokenID accessID_ = 0;
};

class AccessTokenInternetInfo {
public:
    AccessTokenInternetInfo()
    {
        currentID_ = GetSelfTokenID();
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParms, testInternetPolicyPrams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessTokenInternetInfo()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};

class INetConnCallbackTest : public IRemoteStub<INetConnCallback> {
public:
    int32_t NetAvailable(sptr<NetHandle> &netHandle)
    {
        return 0;
    }

    int32_t NetCapabilitiesChange(sptr<NetHandle> &netHandle, const sptr<NetAllCapabilities> &netAllCap)
    {
        return 0;
    }

    int32_t NetConnectionPropertiesChange(sptr<NetHandle> &netHandle, const sptr<NetLinkInfo> &info)
    {
        return 0;
    }

    int32_t NetLost(sptr<NetHandle> &netHandle)
    {
        return 0;
    }

    int32_t NetUnavailable()
    {
        return 0;
    }

    int32_t NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
    {
        return 0;
    }
};

class NetSupplierCallbackBaseTest : public NetSupplierCallbackStub {
};

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
        return false;
    }
    return true;
}

void SystemReadyFuzzTest(const uint8_t *data, size_t size)
{
    AccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetConnService::CMD_NM_SYSTEM_READY, dataParcel);
}

void RegisterNetSupplierFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    uint32_t bearerType = GetData<uint32_t>() % CREATE_NET_TYPE_VALUE;
    dataParcel.ReadUint32(bearerType);

    std::string ident = GetStringFromData(STR_LEN);
    dataParcel.WriteString(ident);

    std::set<NetCap> netCaps{NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    for (auto netCap : netCaps) {
        dataParcel.WriteUint32(static_cast<uint32_t>(netCap));
    }

    OnRemoteRequest(INetConnService::CMD_NM_REG_NET_SUPPLIER, dataParcel);
}

void UnregisterNetSupplierFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    uint32_t supplierId = GetData<uint32_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    OnRemoteRequest(INetConnService::CMD_NM_UNREG_NETWORK, dataParcel);
}

void HasDefaultNetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetConnService::CMD_NM_HASDEFAULTNET, dataParcel);
}

void GetAllNetsFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetConnService::CMD_NM_GET_ALL_NETS, dataParcel);
}

void BindSocketFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    int32_t socket_fd = GetData<int32_t>();
    int32_t netId = GetData<int32_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(socket_fd);
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(INetConnService::CMD_NM_BIND_SOCKET, dataParcel);
}

void SetAirplaneModeFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    bool state = GetData<bool>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteBool(state);
    OnRemoteRequest(INetConnService::CMD_NM_SET_AIRPLANE_MODE, dataParcel);
}

void UpdateNetSupplierInfoFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    uint32_t supplierId = GetData<uint32_t>();
    sptr<NetSupplierInfo> netSupplierInfo = new (std::nothrow) NetSupplierInfo();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    netSupplierInfo->Marshalling(dataParcel);
    OnRemoteRequest(INetConnService::CMD_NM_SET_NET_SUPPLIER_INFO, dataParcel);
}

void GetAddressByNameFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    std::string host = GetStringFromData(STR_LEN);
    int32_t netId = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(host);
    dataParcel.WriteInt32(netId);

    OnRemoteRequest(INetConnService::CMD_NM_GET_ADDRESS_BY_NAME, dataParcel);
}

void GetAddressesByNameFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    std::string host = GetStringFromData(STR_LEN);
    int32_t netId = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(host);
    dataParcel.WriteInt32(netId);

    OnRemoteRequest(INetConnService::CMD_NM_GET_ADDRESSES_BY_NAME, dataParcel);
}

void UpdateNetLinkInfoFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint32_t supplierId = GetData<uint32_t>();
    sptr<NetLinkInfo> netLinkInfo = new (std::nothrow) NetLinkInfo();
    if (netLinkInfo == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    netLinkInfo->Marshalling(dataParcel);

    OnRemoteRequest(INetConnService::CMD_NM_SET_NET_LINK_INFO, dataParcel);
}

void RegisterNetSupplierCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    uint32_t supplierId = GetData<uint32_t>();
    sptr<NetSupplierCallbackBaseTest> callback = new (std::nothrow) NetSupplierCallbackBaseTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(supplierId);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetConnService::CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK, dataParcel);
}

void RegisterNetConnCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    sptr<NetSpecifier> netSpecifier = new (std::nothrow) NetSpecifier();
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    if (netSpecifier == nullptr || callback == nullptr) {
        return;
    }
    uint32_t timeoutMS = GetData<uint32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteUint32(timeoutMS);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetConnService::CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER, dataParcel);
}

void UnregisterNetConnCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    AccessToken token;
    sptr<INetConnCallbackTest> callback = new (std::nothrow) INetConnCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetConnService::CMD_NM_UNREGISTER_NET_CONN_CALLBACK, dataParcel);
}

void GetDefaultNetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetConnService::CMD_NM_GETDEFAULTNETWORK, dataParcel);
}

void GetConnectionPropertiesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    int32_t netId = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(INetConnService::CMD_NM_GET_CONNECTION_PROPERTIES, dataParcel);
}

void GetNetCapabilitiesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    int32_t netId = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(INetConnService::CMD_NM_GET_NET_CAPABILITIES, dataParcel);
}

void NetDetectionFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessTokenInternetInfo tokenInternetInfo;
    int32_t netId = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(INetConnService::CMD_NM_NET_DETECTION, dataParcel);
}

void IsDefaultNetMeteredFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetConnService::CMD_NM_IS_DEFAULT_NET_METERED, dataParcel);
}

void SetGlobalHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    HttpProxy httpProxy = {GetStringFromData(STR_LEN), 0, {}};

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    httpProxy.Marshalling(dataParcel);
    OnRemoteRequest(INetConnService::CMD_NM_SET_HTTP_PROXY, dataParcel);
}

void GetGlobalHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    OnRemoteRequest(INetConnService::CMD_NM_GET_HTTP_PROXY, dataParcel);
}

void GetNetIdByIdentifierFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    std::string ident = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(ident);
    OnRemoteRequest(INetConnService::CMD_NM_GET_NET_ID_BY_IDENTIFIER, dataParcel);
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
    OHOS::NetManagerStandard::GetNetIdByIdentifierFuzzTest(data, size);
    return 0;
}
