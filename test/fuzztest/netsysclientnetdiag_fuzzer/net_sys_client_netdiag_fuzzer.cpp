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

#include "common_net_diag_callback_test.h"
#include "iservice_registry.h"
#include "net_diag_callback_stub.h"
#include "netsys_native_client.h"
#include "notify_callback_stub.h"
#include "singleton.h"
#include "system_ability_definition.h"
#define private public
#include "iptables_wrapper.h"
#include "netsys_native_service.h"
#include "netsys_native_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
constexpr int32_t NUMBER_TWO = 2;
constexpr int32_t NUMBER_ONE = 1;
bool g_isWaitAsync = false;
} // namespace

template <class T> T NetDiagGetData()
{
    T object{};
    size_t netDiagSize = sizeof(object);
    if (g_baseFuzzData == nullptr || netDiagSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, netDiagSize, g_baseFuzzData + g_baseFuzzPos, netDiagSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += netDiagSize;
    return object;
}

std::string NetDiagGetString(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetDiagGetData<char>();
    }
    std::string str(cstr);
    return str;
}

static bool g_isInited = false;
__attribute__((no_sanitize("cfi"))) void Init()
{
    nmd::IptablesWrapper::GetInstance();
    g_isInited = DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->Init();
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    return DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->OnRemoteRequest(code, data, reply,
                                                                                               option);
}

bool WriteInterfaceToken(MessageParcel &data)
{
    return data.WriteInterfaceToken(NetsysNative::NetsysNativeServiceStub::GetDescriptor());
}

bool WriteInterfaceTokenCallback(MessageParcel &data)
{
    return data.WriteInterfaceToken(NetsysNative::NotifyCallbackStub::GetDescriptor());
}

bool IsDataAndSizeValid(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    return WriteInterfaceToken(dataParcel);
}

void NetDiagGetSocketInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    const int maxProtoType = 5;
    NetsysNative::NetDiagProtocolType protoclType =
        static_cast<NetsysNative::NetDiagProtocolType>(NetDiagGetData<uint8_t>() % maxProtoType);
    dataParcel.WriteUint8(static_cast<uint8_t>(protoclType));
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_GET_SOCKETS_INFO),
                    dataParcel);
}

void NetDiagGetRouteTableFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_GET_ROUTE_TABLE),
                    dataParcel);
}

__attribute__((no_sanitize("cfi"))) void NetDiagUpdateInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    bool isAdd = (NetDiagGetData<int32_t>() % NUMBER_TWO == NUMBER_ONE) ? true : false;
    OHOS::NetsysNative::NetDiagIfaceConfig config;
    config.ifaceName_ = NetDiagGetString(STR_LEN);
    config.linkEncap_ = NetDiagGetString(STR_LEN);
    config.macAddr_ = NetDiagGetString(STR_LEN);
    config.ipv4Addr_ = NetDiagGetString(STR_LEN);
    config.ipv4Bcast_ = NetDiagGetString(STR_LEN);
    config.ipv4Mask_ = NetDiagGetString(STR_LEN);
    config.mtu_ = NetDiagGetData<uint32_t>();
    config.txQueueLen_ = NetDiagGetData<uint32_t>();
    config.rxBytes_ = NetDiagGetData<int32_t>();
    config.txBytes_ = NetDiagGetData<int32_t>();

    if (!config.Marshalling(dataParcel)) {
        return;
    }
    dataParcel.WriteString(NetDiagGetString(STR_LEN));
    dataParcel.WriteBool(isAdd);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_UPDATE_IFACE_CONFIG),
                    dataParcel);
}

void NetDiagSetInterfaceActiveFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    const int numberTow = 2;
    std::string iFaceName = NetDiagGetString(STR_LEN);
    bool isUp = NetDiagGetData<uint32_t>() % numberTow == 0;

    dataParcel.WriteString(iFaceName);
    dataParcel.WriteBool(isUp);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_SET_IFACE_ACTIVE_STATE),
                    dataParcel);
}

void NetDiagGetInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string iFaceName = NetDiagGetString(STR_LEN);
    dataParcel.WriteString(iFaceName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_GET_IFACE_CONFIG),
                    dataParcel);
}

__attribute__((no_sanitize("cfi"))) void NetDiagPingFuzzTest(const uint8_t *data, size_t size)
{
    const int maxWaitSecond = 10;
    const int numberTow = 2;
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OHOS::NetsysNative::NetDiagPingOption pingOption;
    pingOption.destination_ = NetDiagGetString(STR_LEN);
    pingOption.source_ = NetDiagGetString(STR_LEN);
    pingOption.count_ = NetDiagGetData<int16_t>();
    pingOption.dataSize_ = NetDiagGetData<int16_t>();
    pingOption.mark_ = NetDiagGetData<int16_t>();
    pingOption.ttl_ = NetDiagGetData<int16_t>();
    pingOption.timeOut_ = NetDiagGetData<int16_t>();
    pingOption.duration_ = NetDiagGetData<int16_t>();
    pingOption.flood_ = NetDiagGetData<int16_t>() % numberTow == 0;

    if (!pingOption.Marshalling(dataParcel)) {
        return;
    }

    sptr<NetsysNative::NetDiagCallbackStubTest> callBack = new NetsysNative::NetDiagCallbackStubTest();
    if (!dataParcel.WriteRemoteObject(callBack->AsObject().GetRefPtr())) {
        return;
    }

    g_isWaitAsync = true;
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_PING_HOST), dataParcel);
    std::chrono::steady_clock::time_point tp1 = std::chrono::steady_clock::now();
    while (g_isWaitAsync) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::chrono::steady_clock::time_point tp2 = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(tp2 - tp1).count() > maxWaitSecond) {
            break;
        }
    }
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::NetDiagGetSocketInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDiagGetRouteTableFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDiagUpdateInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDiagSetInterfaceActiveFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDiagGetInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDiagPingFuzzTest(data, size);
    return 0;
}