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
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;

using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms = {
    .userID = 1,
    .bundleName = "net_conn_client_fuzzer",
    .instIndex = 0,
    .appIDDesc = "test"
};

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

PermissionDef testInternetPermDef = {
    .permissionName = "ohos.permission.INTERNET",
    .bundleName = "net_conn_client_fuzzer",
    .grantMode = 1,
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test net connect maneger internet",
    .descriptionId = 1
};

PermissionStateFull testState = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2}
};

PermissionStateFull testInternetState = {
    .permissionName = "ohos.permission.INTERNET",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {PermissionState::PERMISSION_GRANTED},
    .grantFlags = {2}
};

HapPolicyParams testPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef},
    .permStateList = {testState}
};

HapPolicyParams testInternetPolicyPrams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = {testPermDef, testInternetPermDef},
    .permStateList = {testState, testInternetState}
};
}

template<class T>
T GetData()
{
    T object {};
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

class INetConnCallbackTest : public INetConnCallback {
public:
    INetConnCallbackTest() : INetConnCallback() {}
    virtual ~INetConnCallbackTest() {}
};

class NetSupplierCallbackBaseTest : public NetSupplierCallbackBase {
public:
    NetSupplierCallbackBaseTest() : NetSupplierCallbackBase() {}
    virtual ~NetSupplierCallbackBaseTest() {}
};

void SystemReadyFuzzTest(const uint8_t *data, size_t size)
{
    DelayedSingleton<NetConnClient>::GetInstance()->SystemReady();
}

void RegisterNetSupplierFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string ident = GetStringFromData(STR_LEN);
    std::set<NetCap> netCaps {NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    NetBearType bearerType = BEARER_CELLULAR;
    uint32_t supplierId = GetData<uint32_t>();
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
}

void UnregisterNetSupplierFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint32_t supplierId = GetData<uint32_t>();
    DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(supplierId);
}

void HasDefaultNetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    bool flag = GetData<bool>();
    DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(flag);
}

void GetAllNetsFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    AccessToken token;
    std::list<sptr<NetHandle>> netList;
    DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(netList);
}

void BindSocketFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t socket_fd = GetData<int32_t>();
    int32_t netId = GetData<int32_t>();
    DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socket_fd, netId);
}

void SetAirplaneModeFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    bool state = GetData<bool>();
    DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(state);
}

void UpdateNetSupplierInfoFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint32_t supplierId = GetData<uint32_t>();
    sptr<NetSupplierInfo> netSupplierInfo;
    DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
}

void GetAddressByNameFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    std::string host = GetStringFromData(STR_LEN);
    int32_t netId = GetData<int32_t>();
    INetAddr addr;
    DelayedSingleton<NetConnClient>::GetInstance()->GetAddressByName(host, netId, addr);
}

void GetAddressesByNameFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    std::string host = GetStringFromData(STR_LEN);
    int32_t netId = GetData<int32_t>();
    std::vector<INetAddr> addrList;
    DelayedSingleton<NetConnClient>::GetInstance()->GetAddressesByName(host, netId, addrList);
}

void UpdateNetLinkInfoFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint32_t supplierId = GetData<uint32_t>();
    sptr<NetLinkInfo> netLinkInfo;
    DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
}

void RegisterNetSupplierCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    uint32_t supplierId = GetData<uint32_t>();
    sptr<NetSupplierCallbackBaseTest> callback = sptr<NetSupplierCallbackBaseTest>();
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
}

void RegisterNetConnCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    sptr<NetSpecifier> netSpecifier;
    sptr<INetConnCallbackTest> callback = sptr<INetConnCallbackTest>();
    uint32_t timeoutMS = GetData<uint32_t>();
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback, timeoutMS);
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
}

void UnregisterNetConnCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    AccessToken token;
    sptr<INetConnCallbackTest> callback = sptr<INetConnCallbackTest>();
    DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
}

void GetDefaultNetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    NetHandle netHandle(GetData<int32_t>());
    DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(netHandle);
}

void GetConnectionPropertiesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    NetLinkInfo info;
    NetHandle netHandle(GetData<int32_t>());
    DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(netHandle, info);
}

void GetNetCapabilitiesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token;
    NetAllCapabilities netAllCap;
    NetHandle netHandle(GetData<int32_t>());
    DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(netHandle, netAllCap);
}

void NetDetectionFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessTokenInternetInfo tokenInternetInfo;
    NetHandle netHandle(GetData<int32_t>());
    DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netHandle);
    DelayedSingleton<NetConnClient>::GetInstance()->RestoreFactoryData();
}

void IsDefaultNetMeteredFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    bool isMetered = GetData<int32_t>() % 2 == 0;
    DelayedSingleton<NetConnClient>::GetInstance()->IsDefaultNetMetered(isMetered);
}

void SetHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    std::string httpProxy = GetStringFromData(STR_LEN);
    DelayedSingleton<NetConnClient>::GetInstance()->SetHttpProxy(httpProxy);
}

void GetHttpProxyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token;
    std::string httpProxy = GetStringFromData(STR_LEN);
    DelayedSingleton<NetConnClient>::GetInstance()->GetHttpProxy(httpProxy);
}
} // NetManagerStandard
} // OHOS

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
    OHOS::NetManagerStandard::SetHttpProxyFuzzTest(data, size);
    OHOS::NetManagerStandard::GetHttpProxyFuzzTest(data, size);
    return 0;
}