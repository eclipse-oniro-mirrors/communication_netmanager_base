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

#include <ctime>
#include <thread>
#include <vector>

#include <securec.h>

#include "data_flow_statistics.h"
#include "i_net_stats_service.h"
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
        DelayedSingleton<NetStatsService>::GetInstance()->Init();
        g_isInited = false;
    } else {
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

    int32_t ret = DelayedSingleton<NetStatsService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
    return ret;
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetStatsServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

void RegisterNetStatsCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sptr<INetStatsCallbackTest> callback = new (std::nothrow) INetStatsCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetStatsService::CMD_NSM_REGISTER_NET_STATS_CALLBACK, dataParcel);
}

void UnregisterNetStatsCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sptr<INetStatsCallbackTest> callback = new (std::nothrow) INetStatsCallbackTest();
    if (callback == nullptr) {
        return;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    OnRemoteRequest(INetStatsService::CMD_NSM_UNREGISTER_NET_STATS_CALLBACK, dataParcel);
}

void GetIfaceRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string interfaceName = GetStringFromData(STR_LEN);
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(interfaceName);

    OnRemoteRequest(INetStatsService::CMD_NSM_UNREGISTER_NET_STATS_CALLBACK, dataParcel);
}

void GetIfaceTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string interfaceName = GetStringFromData(STR_LEN);
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(interfaceName);

    OnRemoteRequest(INetStatsService::CMD_GET_IFACE_TXBYTES, dataParcel);
}

void GetUidRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint32_t uid = GetData<uint32_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);

    OnRemoteRequest(INetStatsService::CMD_GET_UID_RXBYTES, dataParcel);
}

void GetUidTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint32_t uid = GetData<uint32_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint32(uid);

    OnRemoteRequest(INetStatsService::CMD_GET_UID_TXBYTES, dataParcel);
}

void GetCellularRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetStatsService::CMD_GET_CELLULAR_RXBYTES, dataParcel);
}

void GetCellularTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetStatsService::CMD_GET_CELLULAR_TXBYTES, dataParcel);
}

void GetAllRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetStatsService::CMD_GET_ALL_RXBYTES, dataParcel);
}

void GetAllTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    OnRemoteRequest(INetStatsService::CMD_GET_ALL_TXBYTES, dataParcel);
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
    return 0;
}