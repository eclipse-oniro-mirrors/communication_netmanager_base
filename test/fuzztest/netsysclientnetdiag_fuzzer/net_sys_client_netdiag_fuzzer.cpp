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

#include <securec.h>
#include <thread>

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
bool g_isWaitAsync = false;
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
class NetDiagCallbackControllerFuzzTest : public IRemoteStub<NetsysNative::INetDiagCallback> {
public:
    NetDiagCallbackControllerFuzzTest()
    {
        memberFuncMap_[static_cast<uint32_t>(NetsysNative::NetDiagInterfaceCode::ON_NOTIFY_PING_RESULT)] =
            &NetDiagCallbackControllerFuzzTest::CmdNotifyPingResult;
    }
    virtual ~NetDiagCallbackControllerFuzzTest() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        NETNATIVE_LOGI("Stub call start, code:[%{public}d]", code);
        std::u16string myDescriptor = NetsysNative::NetDiagCallbackStub::GetDescriptor();
        std::u16string remoteDescriptor = data.ReadInterfaceToken();
        if (myDescriptor != remoteDescriptor) {
            NETNATIVE_LOGE("Descriptor checked failed");
            return NetManagerStandard::NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
        }

        auto itFunc = memberFuncMap_.find(code);
        if (itFunc != memberFuncMap_.end()) {
            auto requestFunc = itFunc->second;
            if (requestFunc != nullptr) {
                return (this->*requestFunc)(data, reply);
            }
        }

        NETNATIVE_LOGI("Stub default case, need check");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    int32_t OnNotifyPingResult(const NetsysNative::NetDiagPingResult &pingResult) override
    {
        g_isWaitAsync = false;
        NETNATIVE_LOGI(
            "OnNotifyPingResult received dateSize_:%{public}d payloadSize_:%{public}d transCount_:%{public}d "
            "recvCount_:%{public}d",
            pingResult.dateSize_, pingResult.payloadSize_, pingResult.transCount_, pingResult.recvCount_);
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

private:
    using NetDiagCallbackFunc = int32_t (NetDiagCallbackControllerFuzzTest::*)(MessageParcel &, MessageParcel &);

private:
    int32_t CmdNotifyPingResult(MessageParcel &data, MessageParcel &reply)
    {
        NetsysNative::NetDiagPingResult pingResult;
        if (!NetsysNative::NetDiagPingResult::Unmarshalling(data, pingResult)) {
            return NetManagerStandard::NETMANAGER_ERR_READ_DATA_FAIL;
        }

        int32_t result = OnNotifyPingResult(pingResult);
        if (!reply.WriteInt32(result)) {
            return NetManagerStandard::NETMANAGER_ERR_WRITE_REPLY_FAIL;
        }
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

private:
    std::map<uint32_t, NetDiagCallbackFunc> memberFuncMap_;
};

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
    const int maxWaitSecond = 10;
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

    if (!pingOption.Marshalling(dataParcel)) {
        return;
    }

    sptr<NetDiagCallbackControllerFuzzTest> callBack = new NetDiagCallbackControllerFuzzTest();

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