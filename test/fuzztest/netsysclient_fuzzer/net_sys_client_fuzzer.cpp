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

bool WriteInterfaceTokenCallback(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetsysNative::NotifyCallbackStub::GetDescriptor())) {
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

void NetworkCreatePhysicalFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    int32_t permission = GetData<int32_t>();

    dataParcel.WriteInt32(netId);
    dataParcel.WriteInt32(permission);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_CREATE_PHYSICAL),
                    dataParcel);
}

void NetworkDestroyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_DESTROY), dataParcel);
}

void NetworkAddInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    std::string iface = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(iface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_ADD_INTERFACE), dataParcel);
}

void NetworkRemoveInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    std::string iface = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(iface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_REMOVE_INTERFACE),
                    dataParcel);
}

void NetworkAddRouteFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    std::string ifName = GetStringFromData(STR_LEN);
    std::string destination = GetStringFromData(STR_LEN);
    std::string nextHop = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(destination);
    dataParcel.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_ADD_ROUTE), dataParcel);
}

void NetworkRemoveRouteFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    std::string ifName = GetStringFromData(STR_LEN);
    std::string destination = GetStringFromData(STR_LEN);
    std::string nextHop = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(ifName);
    dataParcel.WriteString(destination);
    dataParcel.WriteString(nextHop);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_REMOVE_ROUTE), dataParcel);
}

void GetInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    OHOS::nmd::InterfaceConfigurationParcel cfg;
    cfg.ifName = GetStringFromData(STR_LEN);

    dataParcel.WriteString(cfg.ifName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_GET_CONFIG), dataParcel);
}

void GetInterfaceMtuFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string interfaceName = GetStringFromData(STR_LEN);

    dataParcel.WriteString(interfaceName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_GET_MTU), dataParcel);
}

void SetInterfaceMtuFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t mtu = GetData<int32_t>();
    std::string interfaceName = GetStringFromData(STR_LEN);

    dataParcel.WriteString(interfaceName);
    dataParcel.WriteInt32(mtu);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_SET_MTU), dataParcel);
}

void AddInterfaceAddressFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string interfaceName = GetStringFromData(STR_LEN);
    std::string ipAddr = GetStringFromData(STR_LEN);
    int32_t prefixLength = GetData<int32_t>();

    dataParcel.WriteString(interfaceName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_ADD_ADDRESS), dataParcel);
}

class TestNotifyCallback : public NetsysNative::NotifyCallbackStub {
public:
    TestNotifyCallback() = default;
    ~TestNotifyCallback(){};
    int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags, int scope)
    {
        return 0;
    }

    int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags, int scope)
    {
        return 0;
    }

    int32_t OnInterfaceAdded(const std::string &ifName)
    {
        return 0;
    }

    int32_t OnInterfaceRemoved(const std::string &ifName)
    {
        return 0;
    }

    int32_t OnInterfaceChanged(const std::string &ifName, bool up)
    {
        return 0;
    }

    int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up)
    {
        return 0;
    }

    int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                           const std::string &ifName)
    {
        return 0;
    }

    int32_t OnDhcpSuccess(sptr<OHOS::NetsysNative::DhcpResultParcel> &dhcpResult)
    {
        return 0;
    }

    int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface)
    {
        return 0;
    }
};

int32_t OnRemoteRequestCallBack(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    TestNotifyCallback notifyCallBackTest;
    int32_t ret = notifyCallBackTest.OnRemoteRequest(code, data, reply, option);
    return ret;
}

void OnInterfaceAddressUpdatedFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string addr = GetStringFromData(STR_LEN);
    std::string ifName = GetStringFromData(STR_LEN);
    int32_t flags = GetData<int32_t>();
    int32_t scope = GetData<int32_t>();

    MessageParcel dataParcel;
    if (!WriteInterfaceTokenCallback(dataParcel)) {
        return;
    }

    dataParcel.WriteString(addr);
    dataParcel.WriteString(ifName);
    dataParcel.WriteInt32(flags);
    dataParcel.WriteInt32(scope);
    OnRemoteRequestCallBack(static_cast<uint32_t>(NetsysNative::NotifyInterfaceCode::ON_INTERFACE_ADDRESS_UPDATED), dataParcel);
}

void RegisterNotifyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    sptr<NetsysNative::NotifyCallbackStub> notifyCb = new (std::nothrow) TestNotifyCallback();

    notifyCb->Marshalling(dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_REGISTER_NOTIFY_CALLBACK),
                    dataParcel);
}

void UnRegisterNotifyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    sptr<NetsysNative::NotifyCallbackStub> notifyCb = new (std::nothrow) TestNotifyCallback();

    notifyCb->Marshalling(dataParcel);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_UNREGISTER_NOTIFY_CALLBACK),
                    dataParcel);
}

void InterfaceSetIffUpFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ifaceName = GetStringFromData(STR_LEN);

    if (!dataParcel.WriteString("-L -n")) {
        return;
    }

    dataParcel.WriteString(ifaceName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_SET_IFF_UP), dataParcel);
}

void GetAddrInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string serverName = GetStringFromData(STR_LEN);
    AddrInfo hints;
    hints.aiFlags = GetData<uint32_t>();
    hints.aiFamily = GetData<uint32_t>();
    hints.aiSockType = GetData<uint32_t>();
    hints.aiProtocol = GetData<uint32_t>();
    hints.aiAddrLen = GetData<uint32_t>();

    std::string aiCanName = GetStringFromData(STR_LEN);
    if (memcpy_s(hints.aiCanonName, sizeof(hints.aiCanonName), aiCanName.c_str(), aiCanName.length()) != 0) {
        return;
    }
    uint16_t netId = GetData<uint16_t>();
    std::string hostName = GetStringFromData(STR_LEN);

    dataParcel.WriteString(hostName);
    dataParcel.WriteString(serverName);
    dataParcel.WriteRawData(&hints, sizeof(AddrInfo));
    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_ADDR_INFO), dataParcel);
}

void NetworkAddRouteParcelFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    NetsysNative::RouteInfoParcel routInfo;
    routInfo.destination = GetStringFromData(STR_LEN);
    routInfo.ifName = GetStringFromData(STR_LEN);
    routInfo.nextHop = GetStringFromData(STR_LEN);
    routInfo.mtu = GetData<int32_t>();

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(routInfo.destination);
    dataParcel.WriteString(routInfo.ifName);
    dataParcel.WriteString(routInfo.nextHop);
    dataParcel.WriteInt32(routInfo.mtu);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_ADD_ROUTE_PARCEL),
                    dataParcel);
}

void NetworkRemoveRouteParcelFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    NetsysNative::RouteInfoParcel routInfo;
    routInfo.destination = GetStringFromData(STR_LEN);
    routInfo.ifName = GetStringFromData(STR_LEN);
    routInfo.nextHop = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(routInfo.destination);
    dataParcel.WriteString(routInfo.ifName);
    dataParcel.WriteString(routInfo.nextHop);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_REMOVE_ROUTE_PARCEL),
                    dataParcel);
}

void NetworkSetDefaultFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();

    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_SET_DEFAULT), dataParcel);
}

void NetworkGetDefaultFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_GET_DEFAULT), dataParcel);
}
void SetDefaultNetWorkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();

    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_SET_CONFIG), dataParcel);
}

void IpfwdAddInterfaceForwardFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string fromIface = GetStringFromData(STR_LEN);
    std::string toIface = GetStringFromData(STR_LEN);

    dataParcel.WriteString(fromIface);
    dataParcel.WriteString(toIface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_IPFWD_ADD_INTERFACE_FORWARD),
                    dataParcel);
}

void IpfwdRemoveInterfaceForwardFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string fromIface = GetStringFromData(STR_LEN);
    std::string toIface = GetStringFromData(STR_LEN);

    dataParcel.WriteString(fromIface);
    dataParcel.WriteString(toIface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_IPFWD_REMOVE_INTERFACE_FORWARD),
                    dataParcel);
}

void InterfaceSetIpAddressFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ifaceName = GetStringFromData(STR_LEN);
    std::string ipAddress = GetStringFromData(STR_LEN);

    dataParcel.WriteString(ifaceName);
    dataParcel.WriteString(ipAddress);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_SET_IP_ADDRESS),
                    dataParcel);
}

void FirewallSetUidsAllowedListChainFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ifaceName = GetStringFromData(STR_LEN);
    std::string ipAddress = GetStringFromData(STR_LEN);

    dataParcel.WriteString(ifaceName);
    dataParcel.WriteString(ipAddress);
    OnRemoteRequest(
        static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_ALLOWED_LIST_CHAIN),
        dataParcel);
}

void FirewallSetUidsDeniedListChainFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ifaceName = GetStringFromData(STR_LEN);
    std::string ipAddress = GetStringFromData(STR_LEN);

    dataParcel.WriteString(ifaceName);
    dataParcel.WriteString(ipAddress);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_DENIED_LIST_CHAIN),
                    dataParcel);
}

void FirewallSetUidRuleFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t chain = GetData<int32_t>();
    int32_t firewallRule = GetData<int32_t>();

    uint32_t vectorLength = GetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength));
    for (uint32_t i = 0; i <= vectorLength; i++) {
        dataParcel.WriteInt32(GetData<uint32_t>());
    }

    dataParcel.WriteInt32(chain);
    dataParcel.WriteInt32(firewallRule);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_RULE), dataParcel);
}
void SetInterfaceConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    OHOS::nmd::InterfaceConfigurationParcel cfg;
    cfg.ifName = GetStringFromData(STR_LEN);
    cfg.hwAddr = GetStringFromData(STR_LEN);
    cfg.ipv4Addr = GetStringFromData(STR_LEN);
    cfg.prefixLength = GetData<int32_t>();

    uint32_t vectorLength = GetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength));
    for (uint32_t i = 0; i <= vectorLength; i++) {
        dataParcel.WriteString(GetStringFromData(STR_LEN));
    }

    dataParcel.WriteString(cfg.ifName);
    dataParcel.WriteString(cfg.hwAddr);
    dataParcel.WriteString(cfg.ipv4Addr);
    dataParcel.WriteInt32(cfg.prefixLength);
    dataParcel.WriteInt32(cfg.flags.size());
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_SET_CONFIG), dataParcel);
}

void NetworkClearDefaultFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_CLEAR_DEFAULT), dataParcel);
}

void GetProcSysNetFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t family = GetData<int32_t>();
    int32_t which = GetData<int32_t>();
    std::string ifname = GetStringFromData(STR_LEN);
    std::string parameter = GetStringFromData(STR_LEN);
    std::string value = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(family);
    dataParcel.WriteInt32(which);
    dataParcel.WriteString(ifname);
    dataParcel.WriteString(parameter);
    dataParcel.WriteString(value);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_PROC_SYS_NET), dataParcel);
}

void SetProcSysNetFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t family = GetData<int32_t>();
    int32_t which = GetData<int32_t>();
    std::string ifname = GetStringFromData(STR_LEN);
    std::string parameter = GetStringFromData(STR_LEN);
    std::string value = GetStringFromData(STR_LEN);

    dataParcel.WriteInt32(family);
    dataParcel.WriteInt32(which);
    dataParcel.WriteString(ifname);
    dataParcel.WriteString(parameter);
    dataParcel.WriteString(value);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PROC_SYS_NET), dataParcel);
}

void SetInternetPermissionFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t uid = GetData<int32_t>();
    int8_t allow = GetData<int8_t>();

    dataParcel.WriteInt32(uid);
    dataParcel.WriteInt32(allow);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_INTERNET_PERMISSION),
                    dataParcel);
}

void GetFwmarkForNetworkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t netId = GetData<uint32_t>();
    NetsysNative::MarkMaskParcel markParcl;
    markParcl.mark = GetData<int32_t>();
    markParcl.mask = GetData<int32_t>();

    dataParcel.WriteInt32(netId);
    dataParcel.WriteInt32(markParcl.mark);
    dataParcel.WriteInt32(markParcl.mask);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_FWMARK_FOR_NETWORK),
                    dataParcel);
}

void IpEnableForwardingFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string requestor = GetStringFromData(STR_LEN);

    dataParcel.WriteString(requestor);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_IPENABLE_FORWARDING), dataParcel);
}

void IpDisableForwardingFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string requestor = GetStringFromData(STR_LEN);

    dataParcel.WriteString(requestor);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_IPDISABLE_FORWARDING), dataParcel);
}

void EnableNatFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string downstreamIface = GetStringFromData(STR_LEN);
    std::string upstreamIface = GetStringFromData(STR_LEN);

    dataParcel.WriteString(downstreamIface);
    dataParcel.WriteString(upstreamIface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_ENABLE_NAT), dataParcel);
}

void DisableNatFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string downstreamIface = GetStringFromData(STR_LEN);
    std::string upstreamIface = GetStringFromData(STR_LEN);

    dataParcel.WriteString(downstreamIface);
    dataParcel.WriteString(upstreamIface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_DISABLE_NAT), dataParcel);
}

void BandwidthEnableDataSaverFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    bool enable = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    dataParcel.WriteBool(enable);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_ENABLE_DATA_SAVER),
                    dataParcel);
}

void BandwidthSetIfaceQuotaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int64_t bytes = GetData<int64_t>();
    std::string ifName = GetStringFromData(STR_LEN);

    dataParcel.WriteInt64(bytes);
    dataParcel.WriteString(ifName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_SET_IFACE_QUOTA),
                    dataParcel);
}

void BandwidthRemoveIfaceQuotaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ifName = GetStringFromData(STR_LEN);

    dataParcel.WriteString(ifName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_REMOVE_IFACE_QUOTA),
                    dataParcel);
}

void BandwidthAddDeniedListFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t uid = GetData<uint32_t>();

    dataParcel.WriteInt32(uid);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_ADD_DENIED_LIST),
                    dataParcel);
}

void BandwidthRemoveDeniedListFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t uid = GetData<uint32_t>();

    dataParcel.WriteInt32(uid);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_REMOVE_DENIED_LIST),
                    dataParcel);
}

void BandwidthAddAllowedListFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t uid = GetData<uint32_t>();

    dataParcel.WriteInt32(uid);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_ADD_ALLOWED_LIST),
                    dataParcel);
}

void BandwidthRemoveAllowedListFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t uid = GetData<uint32_t>();

    dataParcel.WriteInt32(uid);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_REMOVE_ALLOWED_LIST),
                    dataParcel);
}

void FirewallEnableChainFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t chain = GetData<uint32_t>();
    bool enable = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    dataParcel.WriteInt32(chain);
    dataParcel.WriteBool(enable);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_FIREWALL_ENABLE_CHAIN), dataParcel);
}

void GetNetworkSharingTrafficFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string downIface = GetStringFromData(STR_LEN);
    std::string upIface = GetStringFromData(STR_LEN);
    NetsysNative::NetworkSharingTraffic traffic;
    traffic.receive = GetData<int64_t>();
    traffic.send = GetData<int64_t>();
    traffic.all = GetData<int64_t>();

    dataParcel.WriteString(downIface);
    dataParcel.WriteString(upIface);
    dataParcel.WriteInt64(traffic.receive);
    dataParcel.WriteInt64(traffic.send);
    dataParcel.WriteInt64(traffic.all);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_SHARING_NETWORK_TRAFFIC),
                    dataParcel);
}

void DelInterfaceAddressFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string interfaceName = GetStringFromData(STR_LEN);
    std::string ipAddr = GetStringFromData(STR_LEN);
    int32_t prefixLength = GetData<int32_t>();

    dataParcel.WriteString(interfaceName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_DEL_ADDRESS), dataParcel);
}

void SetResolverConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = GetData<uint16_t>();
    uint16_t baseTimeoutMsec = GetData<uint16_t>();
    uint8_t retryCount = GetData<uint8_t>();

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
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_RESOLVER_CONFIG), dataParcel);
}

void GetResolverConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = GetData<uint16_t>();

    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_RESOLVER_CONFIG), dataParcel);
}

void CreateNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = GetData<uint16_t>();

    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_CREATE_NETWORK_CACHE), dataParcel);
}

void DestroyNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = GetData<uint16_t>();

    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_DESTROY_NETWORK_CACHE), dataParcel);
}

void InterfaceGetListFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_GET_LIST), dataParcel);
}

void ShareDnsSetFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint16_t netId = GetData<uint16_t>();
    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_TETHER_DNS_SET), dataParcel);
}

void StartDnsProxyListenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_START_DNS_PROXY_LISTEN),
                    dataParcel);
}

void StopDnsProxyListenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_STOP_DNS_PROXY_LISTEN), dataParcel);
}

void StartDhcpClientFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string iface = GetStringFromData(STR_LEN);
    bool bIpv6 = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
    dataParcel.WriteString(iface);
    dataParcel.WriteBool(bIpv6);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_START_DHCP_CLIENT), dataParcel);
}

void StopDhcpClientFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string iface = GetStringFromData(STR_LEN);
    bool bIpv6 = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

    dataParcel.WriteString(iface);
    dataParcel.WriteBool(bIpv6);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_STOP_DHCP_CLIENT), dataParcel);
}

void StartDhcpServiceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string iface = GetStringFromData(STR_LEN);
    std::string ipv4addr = GetStringFromData(STR_LEN);
    dataParcel.WriteString(iface);
    dataParcel.WriteString(ipv4addr);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_START_DHCP_SERVICE), dataParcel);
}

void StopDhcpServiceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string iface = GetStringFromData(STR_LEN);
    dataParcel.WriteString(iface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_STOP_DHCP_SERVICE), dataParcel);
}

void GetTotalStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t type = GetData<uint32_t>();
    dataParcel.WriteUint32(type);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_TOTAL_STATS), dataParcel);
}

void GetUidStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint32_t type = GetData<uint32_t>();
    uint32_t uid = GetData<uint32_t>();

    dataParcel.WriteUint32(type);
    dataParcel.WriteUint32(uid);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_UID_STATS), dataParcel);
}

void GetIfaceStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint32_t type = GetData<uint32_t>();
    std::string iface = GetStringFromData(STR_LEN);

    dataParcel.WriteUint32(type);
    dataParcel.WriteString(iface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_IFACE_STATS), dataParcel);
}

void GetAllStatsInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_ALL_STATS_INFO), dataParcel);
}

void SetIptablesCommandForResFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    if (!dataParcel.WriteString("-L -n")) {
        return;
    }

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_IPTABLES_CMD_FOR_RES),
                    dataParcel);
}

void NetworkCreateVirtualFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    bool hasDns = GetData<bool>();

    dataParcel.WriteInt32(netId);
    dataParcel.WriteBool(hasDns);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_CREATE_VIRTUAL),
                    dataParcel);
}

void NetworkAddUidsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    std::vector<UidRange> uidRanges;
    UidRange uid;
    int32_t rangesSize = GetData<int32_t>() % VECTOR_MAX_SIZE;
    for (int i = 0; i < rangesSize; i++) {
        uidRanges.emplace_back(uid);
    }

    dataParcel.WriteInt32(netId);
    dataParcel.WriteInt32(rangesSize);
    for (auto iter : uidRanges) {
        iter.Marshalling(dataParcel);
    }

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_ADD_UIDS), dataParcel);
}

void NetworkDelUidsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = GetData<int32_t>();
    std::vector<UidRange> uidRanges;
    UidRange uid;
    int32_t rangesSize = GetData<int32_t>() % VECTOR_MAX_SIZE;
    for (int i = 0; i < rangesSize; i++) {
        uidRanges.emplace_back(uid);
    }

    dataParcel.WriteInt32(netId);
    dataParcel.WriteInt32(rangesSize);
    for (auto iter : uidRanges) {
        iter.Marshalling(dataParcel);
    }

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_DEL_UIDS), dataParcel);
}

void LLVMFuzzerTestOneInputNew(const uint8_t *data, size_t size)
{
    OHOS::NetManagerStandard::RegisterNotifyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnRegisterNotifyCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceSetIffUpFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAddrInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkAddRouteParcelFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkSetDefaultFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkGetDefaultFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkClearDefaultFuzzTest(data, size);
    OHOS::NetManagerStandard::GetProcSysNetFuzzTest(data, size);
    OHOS::NetManagerStandard::SetProcSysNetFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInternetPermissionFuzzTest(data, size);
    OHOS::NetManagerStandard::GetFwmarkForNetworkFuzzTest(data, size);
    OHOS::NetManagerStandard::IpEnableForwardingFuzzTest(data, size);
    OHOS::NetManagerStandard::IpDisableForwardingFuzzTest(data, size);
    OHOS::NetManagerStandard::EnableNatFuzzTest(data, size);
    OHOS::NetManagerStandard::DisableNatFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthEnableDataSaverFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthSetIfaceQuotaFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthRemoveIfaceQuotaFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthAddDeniedListFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthRemoveDeniedListFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthAddAllowedListFuzzTest(data, size);
    OHOS::NetManagerStandard::BandwidthRemoveAllowedListFuzzTest(data, size);
    OHOS::NetManagerStandard::FirewallEnableChainFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetworkSharingTrafficFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkCreateVirtualFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkAddUidsFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkDelUidsFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkRemoveRouteParcelFuzzTest(data, size);
    OHOS::NetManagerStandard::OnInterfaceAddressUpdatedFuzzTest(data, size);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point1 */

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::NetworkCreatePhysicalFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkDestroyFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkAddInterfaceFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkRemoveInterfaceFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkAddRouteFuzzTest(data, size);
    OHOS::NetManagerStandard::NetworkRemoveRouteFuzzTest(data, size);
    OHOS::NetManagerStandard::GetInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::GetInterfaceMtuFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInterfaceMtuFuzzTest(data, size);
    OHOS::NetManagerStandard::AddInterfaceAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::DelInterfaceAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::SetResolverConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::GetResolverConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::DestroyNetworkCacheFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceGetListFuzzTest(data, size);
    OHOS::NetManagerStandard::ShareDnsSetFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDnsProxyListenFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDnsProxyListenFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDhcpClientFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDhcpClientFuzzTest(data, size);
    OHOS::NetManagerStandard::StartDhcpServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::StopDhcpServiceFuzzTest(data, size);
    OHOS::NetManagerStandard::SetIptablesCommandForResFuzzTest(data, size);
    OHOS::NetManagerStandard::SetDefaultNetWorkFuzzTest(data, size);
    OHOS::NetManagerStandard::SetInterfaceConfigFuzzTest(data, size);
    OHOS::NetManagerStandard::IpfwdAddInterfaceForwardFuzzTest(data, size);
    OHOS::NetManagerStandard::IpfwdRemoveInterfaceForwardFuzzTest(data, size);
    OHOS::NetManagerStandard::InterfaceSetIpAddressFuzzTest(data, size);
    OHOS::NetManagerStandard::FirewallSetUidsAllowedListChainFuzzTest(data, size);
    OHOS::NetManagerStandard::FirewallSetUidsDeniedListChainFuzzTest(data, size);
    OHOS::NetManagerStandard::FirewallSetUidRuleFuzzTest(data, size);
    OHOS::NetManagerStandard::LLVMFuzzerTestOneInputNew(data, size);
    return 0;
}