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
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "i_net_policy_service.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_quota_policy.h"
#define private public
#include "net_policy_service.h"
#include "net_policy_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CREATE_LIMIT_ACTION_VALUE = 2;
static constexpr uint32_t CREATE_NET_TYPE_VALUE = 7;
static constexpr uint32_t CONVERT_NUMBER_TO_BOOL = 2;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
HapInfoParams testInfoParms1 = {.userID = 1,
                                .bundleName = "net_policy_client_fuzzer",
                                .instIndex = 0,
                                .appIDDesc = "test"};

PermissionDef testPermDef1 = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                              .bundleName = "net_policy_client_fuzzer",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState1 = {.permissionName = "ohos.permission.CONNECTIVITY_INTERNAL",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams1 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef1},
                                    .permStateList = {testState1}};

HapInfoParams testInfoParms2 = {.userID = 1,
                                .bundleName = "net_policy_client_fuzzer",
                                .instIndex = 0,
                                .appIDDesc = "test"};

PermissionDef testPermDef2 = {.permissionName = "ohos.permission.SET_NETWORK_POLICY",
                              .bundleName = "net_policy_client_fuzzer",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState2 = {.permissionName = "ohos.permission.SET_NETWORK_POLICY",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams2 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef2},
                                    .permStateList = {testState2}};

HapInfoParams testInfoParms3 = {.userID = 1,
                                .bundleName = "net_policy_client_fuzzer",
                                .instIndex = 0,
                                .appIDDesc = "test"};

PermissionDef testPermDef3 = {.permissionName = "ohos.permission.GET_NETWORK_POLICY",
                              .bundleName = "net_policy_client_fuzzer",
                              .grantMode = 1,
                              .availableLevel = APL_SYSTEM_BASIC,
                              .label = "label",
                              .labelId = 1,
                              .description = "Test net policy connectivity internal",
                              .descriptionId = 1};

PermissionStateFull testState3 = {.permissionName = "ohos.permission.GET_NETWORK_POLICY",
                                  .isGeneral = true,
                                  .resDeviceID = {"local"},
                                  .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                  .grantFlags = {2}};

HapPolicyParams testPolicyPrams3 = {.apl = APL_SYSTEM_BASIC,
                                    .domain = "test.domain",
                                    .permList = {testPermDef3},
                                    .permStateList = {testState3}};
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
    AccessToken(HapInfoParams &testInfoParms, HapPolicyParams &testPolicyPrams) : currentID_(GetSelfTokenID())
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

class INetPolicyCallbackTest : public IRemoteStub<INetPolicyCallback> {
public:
    int32_t NetUidPolicyChange(uint32_t uid, uint32_t policy)
    {
        return 0;
    }

    int32_t NetUidRuleChange(uint32_t uid, uint32_t rule)
    {
        return 0;
    }

    int32_t NetQuotaPolicyChange(const std::vector<NetQuotaPolicy> &quotaPolicies)
    {
        return 0;
    }

    int32_t NetStrategySwitch(const std::string &iccid, bool enable)
    {
        return 0;
    }

    int32_t NetMeteredIfacesChange(std::vector<std::string> &ifaces)
    {
        return 0;
    }

    int32_t NetBackgroundPolicyChange(bool isBackgroundPolicyAllow)
    {
        return 0;
    }
};

static bool g_isInited = false;

void Init()
{
    if (!g_isInited) {
        DelayedSingleton<NetPolicyService>::GetInstance()->Init();
        g_isInited = true;
    }
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    return DelayedSingleton<NetPolicyService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetPolicyServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

void SetPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token(testInfoParms2, testPolicyPrams2);
    uint32_t uid = GetData<uint32_t>();
    uint32_t policy = GetData<uint32_t>() % 3;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);
    dataParcel.WriteUint32(policy);

    OnRemoteRequest(INetPolicyService::CMD_NPS_SET_POLICY_BY_UID, dataParcel);
}

void GetPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token(testInfoParms3, testPolicyPrams3);
    uint32_t uid = GetData<uint32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);

    OnRemoteRequest(INetPolicyService::CMD_NPS_GET_POLICY_BY_UID, dataParcel);
}

void GetUidsByPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token(testInfoParms3, testPolicyPrams3);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    uint32_t policy = GetData<uint32_t>() % 3;
    dataParcel.WriteUint32(policy);

    OnRemoteRequest(INetPolicyService::CMD_NPS_GET_UIDS_BY_POLICY, dataParcel);
}

void SetBackgroundPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token(testInfoParms2, testPolicyPrams2);
    bool isBackgroundPolicyAllow = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteBool(isBackgroundPolicyAllow);
    OnRemoteRequest(INetPolicyService::CMD_NPS_SET_BACKGROUND_POLICY, dataParcel);
}

void GetBackgroundPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token(testInfoParms1, testPolicyPrams1);
    uint32_t uid = GetData<uint32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);
    OnRemoteRequest(INetPolicyService::CMD_NPS_GET_BACKGROUND_POLICY_BY_UID, dataParcel);
}

void SetCellularPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token(testInfoParms2, testPolicyPrams2);

    uint32_t vectorSize = GetData<uint32_t>() % 21;
    std::vector<NetQuotaPolicy> quotaPolicies;
    for (uint32_t i = 0; i < vectorSize; i++) {
        NetQuotaPolicy netQuotaPolicy;
        netQuotaPolicy.netType = GetData<uint32_t>() % CREATE_NET_TYPE_VALUE;

        netQuotaPolicy.iccid = GetStringFromData(STR_LEN);
        netQuotaPolicy.ident = GetStringFromData(STR_LEN);
        netQuotaPolicy.periodStartTime = GetData<int64_t>();
        netQuotaPolicy.periodDuration = GetStringFromData(STR_LEN);

        netQuotaPolicy.warningBytes = GetData<int64_t>();
        netQuotaPolicy.limitBytes = GetData<int64_t>();
        netQuotaPolicy.metered = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
        netQuotaPolicy.limitAction = GetData<uint32_t>() % CREATE_LIMIT_ACTION_VALUE == 0;

        quotaPolicies.push_back(netQuotaPolicy);
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    NetQuotaPolicy::Marshalling(dataParcel, quotaPolicies);

    OnRemoteRequest(INetPolicyService::CMD_NPS_SET_NET_QUOTA_POLICIES, dataParcel);
}

void RegisterNetPolicyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token(testInfoParms1, testPolicyPrams1);
    sptr<INetPolicyCallbackTest> callback = new (std::nothrow) INetPolicyCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetPolicyService::CMD_NPS_REGISTER_NET_POLICY_CALLBACK, dataParcel);
}

void UnregisterNetPolicyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token(testInfoParms1, testPolicyPrams1);
    sptr<INetPolicyCallbackTest> callback = new (std::nothrow) INetPolicyCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetPolicyService::CMD_NPS_UNREGISTER_NET_POLICY_CALLBACK, dataParcel);
}

void GetNetQuotaPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token(testInfoParms3, testPolicyPrams3);
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetPolicyService::CMD_NPS_GET_NET_QUOTA_POLICIES, dataParcel);
}

void SetNetQuotaPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    AccessToken token(testInfoParms2, testPolicyPrams2);

    uint32_t vectorSize = GetData<uint32_t>() % 21;
    std::vector<NetQuotaPolicy> quotaPolicies;
    for (uint32_t i = 0; i < vectorSize; i++) {
        NetQuotaPolicy netQuotaPolicy;
        netQuotaPolicy.netType = GetData<uint32_t>() % CREATE_NET_TYPE_VALUE;

        netQuotaPolicy.iccid = GetStringFromData(STR_LEN);
        netQuotaPolicy.ident = GetStringFromData(STR_LEN);
        netQuotaPolicy.periodStartTime = GetData<int64_t>();
        netQuotaPolicy.periodDuration = GetStringFromData(STR_LEN);

        netQuotaPolicy.warningBytes = GetData<int64_t>();
        netQuotaPolicy.limitBytes = GetData<int64_t>();
        netQuotaPolicy.metered = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
        netQuotaPolicy.limitAction = GetData<uint32_t>() % CREATE_LIMIT_ACTION_VALUE == 0;

        quotaPolicies.push_back(netQuotaPolicy);
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    NetQuotaPolicy::Marshalling(dataParcel, quotaPolicies);

    OnRemoteRequest(INetPolicyService::CMD_NPS_SET_NET_QUOTA_POLICIES, dataParcel);
}

void IsUidNetAllowedFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token(testInfoParms1, testPolicyPrams1);
    uint32_t uid = GetData<uint32_t>();
    bool metered = uid % CONVERT_NUMBER_TO_BOOL == 0;
    std::string ifaceName = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteUint32(uid);
    dataParcel.WriteBool(metered);

    OnRemoteRequest(INetPolicyService::CMD_NPS_IS_NET_ALLOWED_BY_METERED, dataParcel);

    MessageParcel dataParcel2;
    if (!WriteInterfaceToken(dataParcel2)) {
        return;
    }

    dataParcel2.WriteUint32(uid);
    dataParcel2.WriteString(ifaceName);

    OnRemoteRequest(INetPolicyService::CMD_NPS_IS_NET_ALLOWED_BY_IFACE, dataParcel2);
}

void ResetPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    AccessToken token(testInfoParms2, testPolicyPrams2);
    std::string iccid = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteString(iccid);

    OnRemoteRequest(INetPolicyService::CMD_NPS_RESET_POLICIES, dataParcel);
}

void UpdateRemindPolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token(testInfoParms2, testPolicyPrams2);
    int32_t netType = GetData<int32_t>();
    uint32_t remindType = GetData<uint32_t>();
    std::string iccid = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteInt32(netType);
    dataParcel.WriteString(iccid);
    dataParcel.WriteUint32(remindType);

    OnRemoteRequest(INetPolicyService::CMD_NPS_UPDATE_REMIND_POLICY, dataParcel);
}

void SetDeviceIdleAllowedListFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool isAllowed = GetData<int32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    uint32_t uid = GetData<uint32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteUint32(uid);
    dataParcel.WriteBool(isAllowed);

    OnRemoteRequest(INetPolicyService::CMD_NPS_SET_IDLE_ALLOWED_LIST, dataParcel);
}

void SetDeviceIdlePolicyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    AccessToken token(testInfoParms1, testPolicyPrams1);
    bool enable = GetData<int32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteBool(enable);

    OnRemoteRequest(INetPolicyService::CMD_NPS_SET_DEVICE_IDLE_POLICY, dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::SetPolicyByUidFuzzTest(data, size);
    OHOS::NetManagerStandard::GetPolicyByUidFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidsByPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::GetBackgroundPolicyByUidFuzzTest(data, size);
    OHOS::NetManagerStandard::SetCellularPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetPolicyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetQuotaPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::SetNetQuotaPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::IsUidNetAllowedFuzzTest(data, size);
    OHOS::NetManagerStandard::ResetPoliciesFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateRemindPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDeviceIdleAllowedListFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDeviceIdlePolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetPolicyCallbackFuzzTest(data, size);
    return 0;
}
