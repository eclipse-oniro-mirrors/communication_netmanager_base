/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "iservice_registry.h"
#include "notify_callback_stub.h"
#include "singleton.h"
#include "system_ability_definition.h"

#include "netsys_native_client.h"
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

static bool g_isInited = false;
void Init()
{
    nmd::IptablesWrapper::GetInstance();
    if (!DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->Init()) {
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

    int32_t ret =
        DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
    return ret;
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetsysNative::NetsysNativeServiceStub::GetDescriptor())) {
        return false;
    }
    return true;
}

bool WriteInterfaceTokenCallback(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetsysNative::NotifyCallbackStub::GetDescriptor())) {
        return false;
    }
    return true;
}

bool IsDataAndSizeValid(const uint8_t *data, size_t size, MessageParcel &dataParcel)
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

void NetDiagGetSocketInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    NetsysNative::NetDiagProtocolType protoclType =
        static_cast<NetsysNative::NetDiagProtocolType>(GetData<uint8_t>() % 4);
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

void NetDiagUpdateInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    OHOS::NetsysNative::NetDiagIfaceConfig config;
    config.ifaceName_ = GetStringFromData(STR_LEN);
    config.linkEncap_ = GetStringFromData(STR_LEN);
    config.macAddr_ = GetStringFromData(STR_LEN);
    config.ipv4Addr_ = GetStringFromData(STR_LEN);
    config.ipv4Bcast_ = GetStringFromData(STR_LEN);
    config.ipv4Mask_ = GetStringFromData(STR_LEN);
    config.mtu_ = GetData<uint32_t>();
    config.txQueueLen_ = GetData<uint32_t>();
    config.rxBytes_ = GetData<int32_t>();
    config.txBytes_ = GetData<int32_t>();

    if (!config.Marshalling(dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_UPDATE_IFACE_CONFIG),
                    dataParcel);
}

void NetDiagSetInterfaceActiveFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string iFaceName = GetStringFromData(STR_LEN);
    bool isUp = GetData<uint32_t>() % 2 == 0 ? true : false;

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

    std::string iFaceName = GetStringFromData(STR_LEN);
    dataParcel.WriteString(iFaceName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_GET_IFACE_CONFIG),
                    dataParcel);
}

void NetDiagPingFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OHOS::NetsysNative::NetDiagPingOption pingOption;
    pingOption.destination_ = GetStringFromData(STR_LEN);
    pingOption.source_ = GetStringFromData(STR_LEN);
    pingOption.count_ = GetData<int16_t>();
    pingOption.dataSize_ = GetData<int16_t>();
    pingOption.mark_ = GetData<int16_t>();
    pingOption.ttl_ = GetData<int16_t>();
    pingOption.timeOut_ = GetData<int16_t>();
    pingOption.duration_ = GetData<int16_t>();
    pingOption.flood_ = GetData<int16_t>() % 2 == 0 ? true : false;
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETDIAG_PING_HOST), dataParcel);
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