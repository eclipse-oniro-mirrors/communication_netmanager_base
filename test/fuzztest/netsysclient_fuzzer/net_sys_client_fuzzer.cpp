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

#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

#include "netsys_native_client.h"
#define private public
#include "netsys_native_service.h"
#include "netsys_native_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
static constexpr uint32_t CONVERT_NUMBER_TO_BOOL = 2;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
constexpr size_t VECTOR_MAX_SIZE = 15;
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

class INetSysCallbackTest : public NetsysNative::NotifyCallbackStub {
public:
    INetSysCallbackTest() : NotifyCallbackStub() {}
    virtual ~INetSysCallbackTest() {}
};

class NetsysControllerCallbackTest : public NetsysControllerCallback {
public:
    virtual int32_t OnInterfaceAddressUpdated(const std::string &, const std::string &, int, int)
    {
        return 0;
    }
    virtual int32_t OnInterfaceAddressRemoved(const std::string &, const std::string &, int, int)
    {
        return 0;
    }
    virtual int32_t OnInterfaceAdded(const std::string &)
    {
        return 0;
    }
    virtual int32_t OnInterfaceRemoved(const std::string &)
    {
        return 0;
    }
    virtual int32_t OnInterfaceChanged(const std::string &, bool)
    {
        return 0;
    }
    virtual int32_t OnInterfaceLinkStateChanged(const std::string &, bool)
    {
        return 0;
    }
    virtual int32_t OnRouteChanged(bool, const std::string &, const std::string &, const std::string &)
    {
        return 0;
    }
    virtual int32_t OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
    {
        return 0;
    }
    virtual int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface)
    {
        return 0;
    }
};

static NetsysNative::NetsysNativeService g_netSysNativeClient;

void NetworkCreatePhysicalFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t netId = GetData<int32_t>();
    int32_t permission = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteInt32(permission);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_NETWORK_CREATE_PHYSICAL, dataParcel);
}

void NetworkDestroyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t netId = GetData<int32_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_NETWORK_DESTROY, dataParcel);
}

void NetworkAddInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t netId = GetData<int32_t>();
    std::string iface = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(iface);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_NETWORK_ADD_INTERFACE, dataParcel);
}

void NetworkRemoveInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t netId = GetData<int32_t>();
    std::string iface = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(iface);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_NETWORK_REMOVE_INTERFACE, dataParcel);
}

void NetworkAddRouteFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t netId = GetData<int32_t>();
    std::string ifName = GetStringFromData(STR_LEN);
    std::string destination = GetStringFromData(STR_LEN);
    std::string nextHop = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(destination);
    dataParcel.WriteString(nextHop);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_NETWORK_ADD_ROUTE, dataParcel);
}

void NetworkRemoveRouteFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t netId = GetData<int32_t>();
    std::string ifName = GetStringFromData(STR_LEN);
    std::string destination = GetStringFromData(STR_LEN);
    std::string nextHop = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(destination);
    dataParcel.WriteString(nextHop);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_NETWORK_REMOVE_ROUTE, dataParcel);
}

void InterfaceGetConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    OHOS::nmd::InterfaceConfigurationParcel cfg;
    cfg.ifName = GetStringFromData(STR_LEN);
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }

    dataParcel.WriteString(cfg.ifName);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_INTERFACE_GET_CONFIG, dataParcel);
}

void InterfaceGetMtuFuzzTest(const uint8_t *data, size_t size)
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
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_INTERFACE_GET_MTU, dataParcel);
}

void InterfaceSetMtuFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    int32_t mtu = GetData<int32_t>();
    std::string interfaceName = GetStringFromData(STR_LEN);

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(interfaceName);
    dataParcel.WriteInt32(mtu);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_INTERFACE_SET_MTU, dataParcel);
}

void InterfaceAddAddressFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string interfaceName = GetStringFromData(STR_LEN);
    std::string ipAddr = GetStringFromData(STR_LEN);
    int32_t prefixLength = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(interfaceName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_INTERFACE_ADD_ADDRESS, dataParcel);
}

void InterfaceDelAddressFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string interfaceName = GetStringFromData(STR_LEN);
    std::string ipAddr = GetStringFromData(STR_LEN);
    int32_t prefixLength = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(interfaceName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_INTERFACE_DEL_ADDRESS, dataParcel);
}

void SetResolverConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint16_t netId = GetData<uint16_t>();
    uint16_t baseTimeoutMsec = GetData<uint16_t>();
    uint8_t retryCount = GetData<uint8_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint16(netId);
    dataParcel.WriteUint16(baseTimeoutMsec);
    dataParcel.WriteUint8(retryCount);

    uint32_t vectorLength = GetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength));
    for (uint32_t i = 0; i <= vectorLength; i++) {
        dataParcel.WriteString(GetStringFromData(STR_LEN));
    }

    uint32_t vectorLength2 = GetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength2));
    for (uint32_t i = 0; i <= vectorLength2; i++) {
        dataParcel.WriteString(GetStringFromData(STR_LEN));
    }
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_SET_RESOLVER_CONFIG, dataParcel);
}

void GetResolverConfigFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint16_t netId = GetData<uint16_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint16(netId);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_GET_RESOLVER_CONFIG, dataParcel);
}

void CreateNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint16_t netId = GetData<uint16_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint16(netId);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_CREATE_NETWORK_CACHE, dataParcel);
}

void DestroyNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint16_t netId = GetData<uint16_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint16(netId);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_DESTROY_NETWORK_CACHE, dataParcel);
}

void InterfaceGetListFuzzTest(const uint8_t *data, size_t size)
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
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_INTERFACE_GET_LIST, dataParcel);
}

void ShareDnsSetFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint16_t netId = GetData<uint16_t>();
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteUint16(netId);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_TETHER_DNS_SET, dataParcel);
}

void StartDnsProxyListenFuzzTest(const uint8_t *data, size_t size)
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
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_START_DNS_PROXY_LISTEN, dataParcel);
}

void StopDnsProxyListenFuzzTest(const uint8_t *data, size_t size)
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
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_STOP_DNS_PROXY_LISTEN, dataParcel);
}

void StartDhcpClientFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    bool bIpv6 = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(iface);
    dataParcel.WriteBool(bIpv6);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_START_DHCP_CLIENT, dataParcel);
}

void StopDhcpClientFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    bool bIpv6 = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(iface);
    dataParcel.WriteBool(bIpv6);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_STOP_DHCP_CLIENT, dataParcel);
}

void StartDhcpServiceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    std::string ipv4addr = GetStringFromData(STR_LEN);
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(iface);
    dataParcel.WriteString(ipv4addr);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_START_DHCP_SERVICE, dataParcel);
}

void StopDhcpServiceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        return;
    }
    dataParcel.WriteString(iface);
    OnRemoteRequest(NetsysNative::INetsysService::NETSYS_STOP_DHCP_SERVICE, dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::NetworkCreatePhysicalFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkDestroyFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkAddInterfaceFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkRemoveInterfaceFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkAddRouteFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkRemoveRouteFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceGetConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceGetMtuFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceSetMtuFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceAddAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceDelAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::SetResolverConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::GetResolverConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::DestroyNetworkCacheFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceGetListFuzzTest(data, size);
    OHOS::NetManagerStandard::ShareDnsSetFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDnsProxyListenFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDhcpClientFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDhcpClientFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDhcpServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDhcpServiceFuzzTest(data, size);
    return 0;
}