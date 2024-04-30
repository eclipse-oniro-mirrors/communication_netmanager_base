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

#include "i_net_policy_service.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_client.h"
#include "net_policy_constants.h"
#include "net_quota_policy.h"
#include "netmanager_base_test_security.h"
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
} // namespace

template<class T>
T NetPolicyGetData()
{
    T object {};
    size_t netPolicySize = sizeof(object);
    if (g_baseFuzzData == nullptr || netPolicySize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, netPolicySize, g_baseFuzzData + g_baseFuzzPos, netPolicySize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += netPolicySize;
    return object;
}

std::string NetPolicyGetString(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetPolicyGetData<char>();
    }
    std::string str(cstr);
    return str;
}

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

    int32_t NetStrategySwitch(const std::string &simId, bool enable)
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

bool IsValidPolicyFuzzData(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if ((data == nullptr) || (size == 0)) {
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

void SetPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    uint32_t uid = NetPolicyGetData<uint32_t>();
    uint32_t policy = NetPolicyGetData<uint32_t>() % 3;
    dataParcel.WriteUint32(uid);
    dataParcel.WriteUint32(policy);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POLICY_BY_UID), dataParcel);
}

void GetPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    uint32_t uid = NetPolicyGetData<uint32_t>();

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_POLICY_BY_UID), dataParcel);
}

void GetUidsByPolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    uint32_t policy = NetPolicyGetData<uint32_t>() % 3;
    dataParcel.WriteUint32(policy);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_UIDS_BY_POLICY), dataParcel);
}

void SetBackgroundPolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    bool isBackgroundPolicyAllow = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteBool(isBackgroundPolicyAllow);
    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_BACKGROUND_POLICY), dataParcel);
}

void GetBackgroundPolicyByUidFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    uint32_t uid = NetPolicyGetData<uint32_t>();

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);
    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_BACKGROUND_POLICY_BY_UID), dataParcel);
}

void SetCellularPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;

    uint32_t vectorSize = NetPolicyGetData<uint32_t>() % 21;
    std::vector<NetQuotaPolicy> quotaPolicies;
    for (uint32_t i = 0; i < vectorSize; i++) {
        NetQuotaPolicy netQuotaPolicy;
        netQuotaPolicy.networkmatchrule.netType = NetPolicyGetData<uint32_t>() % CREATE_NET_TYPE_VALUE;

        netQuotaPolicy.networkmatchrule.simId = NetPolicyGetString(STR_LEN);
        netQuotaPolicy.networkmatchrule.ident = NetPolicyGetString(STR_LEN);
        netQuotaPolicy.quotapolicy.periodStartTime = NetPolicyGetData<int64_t>();
        netQuotaPolicy.quotapolicy.periodDuration = NetPolicyGetString(STR_LEN);

        netQuotaPolicy.quotapolicy.warningBytes = NetPolicyGetData<int64_t>();
        netQuotaPolicy.quotapolicy.limitBytes = NetPolicyGetData<int64_t>();
        netQuotaPolicy.quotapolicy.metered = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
        netQuotaPolicy.quotapolicy.limitAction = NetPolicyGetData<uint32_t>() % CREATE_LIMIT_ACTION_VALUE == 0;

        quotaPolicies.push_back(netQuotaPolicy);
    }

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    NetQuotaPolicy::Marshalling(dataParcel, quotaPolicies);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NET_QUOTA_POLICIES), dataParcel);
}

void RegisterNetPolicyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    sptr<INetPolicyCallbackTest> callback = new (std::nothrow) INetPolicyCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_REGISTER_NET_POLICY_CALLBACK), dataParcel);
}

void UnregisterNetPolicyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    sptr<INetPolicyCallbackTest> callback = new (std::nothrow) INetPolicyCallbackTest();
    if (callback == nullptr) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_UNREGISTER_NET_POLICY_CALLBACK), dataParcel);
}

void GetNetQuotaPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_NET_QUOTA_POLICIES), dataParcel);
}

void SetNetQuotaPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    uint32_t vectorSize = NetPolicyGetData<uint32_t>() % 21;
    std::vector<NetQuotaPolicy> quotaPolicies;
    for (uint32_t i = 0; i < vectorSize; i++) {
        NetQuotaPolicy netQuotaPolicy;
        netQuotaPolicy.networkmatchrule.netType = NetPolicyGetData<uint32_t>() % CREATE_NET_TYPE_VALUE;

        netQuotaPolicy.networkmatchrule.simId = NetPolicyGetString(STR_LEN);
        netQuotaPolicy.networkmatchrule.ident = NetPolicyGetString(STR_LEN);
        netQuotaPolicy.quotapolicy.periodStartTime = NetPolicyGetData<int64_t>();
        netQuotaPolicy.quotapolicy.periodDuration = NetPolicyGetString(STR_LEN);

        netQuotaPolicy.quotapolicy.warningBytes = NetPolicyGetData<int64_t>();
        netQuotaPolicy.quotapolicy.limitBytes = NetPolicyGetData<int64_t>();
        netQuotaPolicy.quotapolicy.metered = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
        netQuotaPolicy.quotapolicy.limitAction = NetPolicyGetData<uint32_t>() % CREATE_LIMIT_ACTION_VALUE == 0;

        quotaPolicies.push_back(netQuotaPolicy);
    }
    NetQuotaPolicy::Marshalling(dataParcel, quotaPolicies);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NET_QUOTA_POLICIES), dataParcel);
}

void IsUidNetAllowedFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    uint32_t uid = NetPolicyGetData<uint32_t>();
    bool metered = uid % CONVERT_NUMBER_TO_BOOL == 0;
    std::string ifaceName = NetPolicyGetString(STR_LEN);
    dataParcel.WriteUint32(uid);
    dataParcel.WriteBool(metered);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_IS_NET_ALLOWED_BY_METERED), dataParcel);

    MessageParcel dataParcel2;
    if (!WriteInterfaceToken(dataParcel2)) {
        return;
    }

    dataParcel2.WriteUint32(uid);
    dataParcel2.WriteString(ifaceName);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_IS_NET_ALLOWED_BY_IFACE), dataParcel2);
}

void ResetPoliciesFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    std::string simId = NetPolicyGetString(STR_LEN);

    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    dataParcel.WriteString(simId);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_RESET_POLICIES), dataParcel);
}

void UpdateRemindPolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    int32_t netType = NetPolicyGetData<int32_t>();
    uint32_t remindType = NetPolicyGetData<uint32_t>();
    std::string simId = NetPolicyGetString(STR_LEN);
    dataParcel.WriteInt32(netType);
    dataParcel.WriteString(simId);
    dataParcel.WriteUint32(remindType);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_UPDATE_REMIND_POLICY), dataParcel);
}

void SetDeviceIdleTrustlistFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    bool isAllowed = NetPolicyGetData<int32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    std::vector<uint32_t> uids = NetPolicyGetData<std::vector<uint32_t>>();
    dataParcel.WriteUInt32Vector(uids);
    dataParcel.WriteBool(isAllowed);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_IDLE_TRUSTLIST), dataParcel);
}

void SetDeviceIdlePolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    bool enable = NetPolicyGetData<int32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    dataParcel.WriteBool(enable);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_DEVICE_IDLE_POLICY), dataParcel);
}

void SetPowerSaveTrustlistFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    bool isAllowed = NetPolicyGetData<int32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    std::vector<uint32_t> uids = NetPolicyGetData<std::vector<uint32_t>>();
    dataParcel.WriteBool(isAllowed);
    dataParcel.WriteUInt32Vector(uids);

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POWER_SAVE_TRUSTLIST), dataParcel);
}

void GetPowerSaveTrustlistFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_POWER_SAVE_TRUSTLIST), dataParcel);
}

void GetDeviceIdleTrustlistFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_IDLE_TRUSTLIST), dataParcel);
}

void GetBackgroundPolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_BACKGROUND_POLICY), dataParcel);
}

void SetPowerSavePolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    bool enable = NetPolicyGetData<int32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    dataParcel.WriteBool(enable);
    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_POWER_SAVE_POLICY), dataParcel);
}

void CheckPermissionFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_CHECK_PERMISSION), dataParcel);
}

/**
 * @tc.name: SetNetworkAccessPolicy001
 * @tc.desc: Test NetPolicyClient SetNetworkAccessPolicy.
 * @tc.type: FUNC
 */
void SetNetworkAccessPolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }
    NetworkAccessPolicy netAccessPolicy;
    uint32_t uid = NetPolicyGetData<uint32_t>();
    netAccessPolicy.wifiAllow = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL;
    netAccessPolicy.cellularAllow = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL;
    bool reconfirmFlag = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL;

    dataParcel.WriteUint32(uid);
    dataParcel.WriteUint8(netAccessPolicy.wifiAllow);
    dataParcel.WriteUint8(netAccessPolicy.cellularAllow);
    dataParcel.WriteBool(reconfirmFlag);
    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_SET_NETWORK_ACCESS_POLICY), dataParcel);
}

/**
 * @tc.name: GetNetworkAccessPolicy001
 * @tc.desc: Test NetPolicyClient GetNetworkAccessPolicy.
 * @tc.type: FUNC
 */
void GetNetworkAccessPolicyFuzzTest(const uint8_t *data, size_t size)
{
    NetManagerBaseAccessToken token;
    MessageParcel dataParcel;
    if (!IsValidPolicyFuzzData(data, size, dataParcel)) {
        return;
    }

    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    bool flag = NetPolicyGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL;
    uint32_t uid = NetPolicyGetData<uint32_t>();
    uint32_t userId = NetPolicyGetData<uint32_t>();
    dataParcel.WriteBool(flag);
    dataParcel.WriteInt32(uid);
    dataParcel.WriteUint32(userId);
    OnRemoteRequest(static_cast<uint32_t>(PolicyInterfaceCode::CMD_NPS_GET_NETWORK_ACCESS_POLICY), dataParcel);
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
    OHOS::NetManagerStandard::SetDeviceIdleTrustlistFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDeviceIdlePolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPowerSaveTrustlistFuzzTest(data, size);
    OHOS::NetManagerStandard::GetPowerSaveTrustlistFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetPolicyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetDeviceIdleTrustlistFuzzTest(data, size);
    OHOS::NetManagerStandard::GetBackgroundPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPowerSavePolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::CheckPermissionFuzzTest(data, size);
    OHOS::NetManagerStandard::SetNetworkAccessPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetworkAccessPolicyFuzzTest(data, size);
    return 0;
}