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

#include <ctime>
#include <thread>
#include <vector>

#include <securec.h>

#include "data_flow_statistics.h"
#include "inet_stats_service.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"
#define private public
#include "net_stats_service.h"
#include "net_stats_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
} // namespace

template <class T> T NetStatsGetData()
{
    T object{};
    size_t netStatsSize = sizeof(object);
    if (g_baseFuzzData == nullptr || netStatsSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, netStatsSize, g_baseFuzzData + g_baseFuzzPos, netStatsSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += netStatsSize;
    return object;
}

std::string NetStatsGetString(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetStatsGetData<char>();
    }
    std::string str(cstr);
    return str;
}

class INetStatsCallbackTest : public IRemoteStub<INetStatsCallback> {
public:
    int32_t NetIfaceStatsChanged(const std::string &iface)
    {
        return 0;
    }

    int32_t NetUidStatsChanged(const std::string &iface, uint32_t uid)
    {
        return 0;
    }
};

static bool g_isInited = false;

void Init()
{
    if (!g_isInited) {
        NetStatsService::GetInstance()->Init();
        g_isInited = true;
    } else {
        g_isInited = false;
    }
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    int32_t ret = NetStatsService::GetInstance()->OnRemoteRequest(code, data, reply, option);
    return ret;
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

void CheckParamVaild(MessageParcel &dataParcel, const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    return;
}

void RegisterNetStatsCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    sptr<INetStatsCallbackTest> callback = new (std::nothrow) INetStatsCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    // CheckParamVaild 内部已写 interface token，此处不得再显式写一次：
    // 原实现为 [token][token][RemoteObject]，stub 的 ReadRemoteObject 读到第二个
    // token 而非回调对象，iface_cast 失败早退（Round A 实证仅 1 次穿透）
    CheckParamVaild(dataParcel, data, size);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_REGISTER_NET_STATS_CALLBACK), dataParcel);
}

void UnregisterNetStatsCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    sptr<INetStatsCallbackTest> callback = new (std::nothrow) INetStatsCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    // 统一为 [token(由 CheckParamVaild 内写)][RemoteObject] 的净序：
    // 原实现把 token 追加在 RemoteObject 之后形成尾部冗余（stub 忽略尾部、
    // 恰好能工作，Round A 实证 24 次命中），此处仅清理为规范序
    CheckParamVaild(dataParcel, data, size);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_UNREGISTER_NET_STATS_CALLBACK), dataParcel);
}

__attribute__((no_sanitize("cfi"))) void GetIfaceRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    std::string interfaceName = NetStatsGetString(STR_LEN);
    dataParcel.WriteString(interfaceName);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_IFACE_RX_BYTES), dataParcel);
}

void GetIfaceTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    std::string interfaceName = NetStatsGetString(STR_LEN);
    dataParcel.WriteString(interfaceName);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_IFACE_TX_BYTES), dataParcel);
}

void GetUidRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint32_t uid = NetStatsGetData<uint32_t>();
    dataParcel.WriteUint32(uid);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_UID_RX_BYTES), dataParcel);
}

void GetUidTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint32_t uid = NetStatsGetData<uint32_t>();
    dataParcel.WriteUint32(uid);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_UID_TX_BYTES), dataParcel);
}

void GetCellularRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_CELLULAR_RX_BYTES), dataParcel);
}

void GetCellularTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_CELLULAR_TX_BYTES), dataParcel);
}

void GetAllRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_ALL_RX_BYTES), dataParcel);
}

void GetAllTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_ALL_TX_BYTES), dataParcel);
}

void GetIfaceStatsDetailFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    dataParcel.WriteString(NetStatsGetString(STR_LEN));
    dataParcel.WriteUint64(NetStatsGetData<uint64_t>());
    dataParcel.WriteUint64(NetStatsGetData<uint64_t>());

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_IFACE_STATS_DETAIL), dataParcel);
}

void GetUidStatsDetailFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    dataParcel.WriteString(NetStatsGetString(STR_LEN));
    dataParcel.WriteUint64(NetStatsGetData<uint32_t>());
    dataParcel.WriteUint64(NetStatsGetData<uint64_t>());
    dataParcel.WriteUint64(NetStatsGetData<uint64_t>());

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_UID_STATS_DETAIL), dataParcel);
}

void UpdateIfacesStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    dataParcel.WriteString(NetStatsGetString(STR_LEN));
    dataParcel.WriteUint64(NetStatsGetData<uint64_t>());
    dataParcel.WriteUint64(NetStatsGetData<uint64_t>());
    NetStatsInfo stats;
    stats.iface_ = NetStatsGetString(STR_LEN);
    stats.uid_ = NetStatsGetData<uint32_t>();
    stats.date_ = NetStatsGetData<uint64_t>();
    stats.rxBytes_ = NetStatsGetData<uint64_t>();
    stats.txBytes_ = NetStatsGetData<uint64_t>();
    stats.rxPackets_ = NetStatsGetData<uint64_t>();
    stats.txPackets_ = NetStatsGetData<uint64_t>();
    // ReadParcelable 需先读 int32 size 前缀，必须用 WriteParcelable 而非裸 Marshalling
    dataParcel.WriteParcelable(&stats);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_UPDATE_IFACES_STATS), dataParcel);
}

void UpdateStatsDataFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_UPDATE_STATS_DATA), dataParcel);
}

void ResetFactoryFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_RESET_FACTORY), dataParcel);
}

void GetCookieRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint64_t cookie = NetStatsGetData<uint64_t>();
    dataParcel.WriteUint32(cookie);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_COOKIE_RX_BYTES), dataParcel);
}

void GetCookieTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint64_t cookie = NetStatsGetData<uint64_t>();
    dataParcel.WriteUint32(cookie);

    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_COOKIE_TX_BYTES), dataParcel);
}

void SetAppStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    PushStatsInfo info;
    info.uid_ = NetStatsGetData<uint32_t>();
    info.iface_ = NetStatsGetString(STR_LEN);
    info.rxBytes_ = NetStatsGetData<uint64_t>();
    info.txBytes_ = NetStatsGetData<uint64_t>();
    dataParcel.WriteParcelable(&info);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_SET_APP_STATS), dataParcel);
}

void GetTrafficStatsByNetworkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    // stub 端 ReadParcelable<NetStatsNetwork>：需 WriteParcelable 写 size 前缀 + 字段
    NetStatsNetwork network;
    network.type_ = NetStatsGetData<uint32_t>();
    network.startTime_ = NetStatsGetData<uint64_t>();
    network.endTime_ = NetStatsGetData<uint64_t>();
    network.simId_ = NetStatsGetData<uint32_t>();
    dataParcel.WriteParcelable(&network);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_TRAFFIC_STATS_BY_NETWORK), dataParcel);
}

void GetTrafficStatsByUidNetworkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint32_t uid = NetStatsGetData<uint32_t>();
    NetStatsNetwork network;
    network.type_ = NetStatsGetData<uint32_t>();
    network.startTime_ = NetStatsGetData<uint64_t>();
    network.endTime_ = NetStatsGetData<uint64_t>();
    network.simId_ = NetStatsGetData<uint32_t>();
    dataParcel.WriteUint32(uid);
    dataParcel.WriteParcelable(&network);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_TRAFFIC_STATS_BY_UID_NETWORK), dataParcel);
}

void GetMonthTrafficStatsByNetworkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint32_t simId = NetStatsGetData<uint32_t>();
    uint32_t usedData = NetStatsGetData<uint64_t>();
    dataParcel.WriteUint32(simId);
    dataParcel.WriteUint64(usedData);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_MONTH_TRAFFIC_STATS_BY_NETWORK), dataParcel);
}

void SetCalibrationTrafficFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    uint32_t simId = NetStatsGetData<uint32_t>();
    int64_t remainData = NetStatsGetData<int64_t>();
    uint64_t tatalData = NetStatsGetData<uint64_t>();
    dataParcel.WriteUint32(simId);
    dataParcel.WriteInt64(remainData);
    dataParcel.WriteUint64(tatalData);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_SET_CALIBRATION_TRAFFIC), dataParcel);
}

void SetTrafficPlanInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    int32_t simId = NetStatsGetData<int32_t>();
    int32_t param = NetStatsGetData<int32_t>();
    int64_t value = NetStatsGetData<int64_t>();
    dataParcel.WriteInt32(simId);
    dataParcel.WriteInt32(param);
    dataParcel.WriteInt64(value);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_SET_TRAFFIC_PLAN_INFO), dataParcel);
}

void GetTrafficPlanInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    int32_t simId = NetStatsGetData<int32_t>();
    int32_t param = NetStatsGetData<int32_t>();
    dataParcel.WriteInt32(simId);
    dataParcel.WriteInt32(param);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_TRAFFIC_PLAN_INFO), dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

namespace OHOS {
namespace NetManagerStandard {
void GetAllStatsInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_ALL_STATS_INFO), dataParcel);
}

void GetAllSimStatsInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_GET_ALL_SIM_STATS_INFO), dataParcel);
}

void SetDpaAppStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    // stub 端 ReadParcelable<NetStatsInfo>：需 WriteParcelable 写 size 前缀 + 字段
    NetStatsInfo info;
    info.iface_ = NetStatsGetString(STR_LEN);
    info.uid_ = NetStatsGetData<uint32_t>();
    info.date_ = NetStatsGetData<uint64_t>();
    info.rxBytes_ = NetStatsGetData<uint64_t>();
    info.txBytes_ = NetStatsGetData<uint64_t>();
    dataParcel.WriteParcelable(&info);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_SET_DPA_APP_STATS), dataParcel);
}

void SaveSharingTrafficFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamVaild(dataParcel, data, size);
    // stub 端 ReadParcelable<NetStatsInfo>：需 WriteParcelable 写 size 前缀 + 字段
    NetStatsInfo infos;
    infos.iface_ = NetStatsGetString(STR_LEN);
    infos.uid_ = NetStatsGetData<uint32_t>();
    infos.date_ = NetStatsGetData<uint64_t>();
    infos.rxBytes_ = NetStatsGetData<uint64_t>();
    infos.txBytes_ = NetStatsGetData<uint64_t>();
    dataParcel.WriteParcelable(&infos);
    OnRemoteRequest(static_cast<uint32_t>(INetStatsServiceIpcCode::COMMAND_SAVE_SHARING_TRAFFIC), dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::RegisterNetStatsCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetStatsCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetCellularRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetCellularTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceStatsDetailFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidStatsDetailFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateIfacesStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateStatsDataFuzzTest(data, size);
    OHOS::NetManagerStandard::ResetFactoryFuzzTest(data, size);
    OHOS::NetManagerStandard::GetCookieRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetCookieTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::SetAppStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::GetTrafficStatsByNetworkFuzzTest(data, size);
    OHOS::NetManagerStandard::GetTrafficStatsByUidNetworkFuzzTest(data, size);
    OHOS::NetManagerStandard::GetMonthTrafficStatsByNetworkFuzzTest(data, size);
    OHOS::NetManagerStandard::SetCalibrationTrafficFuzzTest(data, size);
    OHOS::NetManagerStandard::SetTrafficPlanInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::GetTrafficPlanInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllStatsInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllSimStatsInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDpaAppStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::SaveSharingTrafficFuzzTest(data, size);
    return 0;
}
