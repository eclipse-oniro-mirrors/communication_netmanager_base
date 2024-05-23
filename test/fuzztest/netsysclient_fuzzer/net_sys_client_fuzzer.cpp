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

#include "common_notify_callback_test.h"
#include "iservice_registry.h"
#include "net_dns_health_callback_stub.h"
#include "net_dns_result_callback_stub.h"
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
static constexpr uint32_t CONVERT_NUMBER_TO_BOOL = 2;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
constexpr size_t VECTOR_MAX_SIZE = 15;
} // namespace

template <class T> T NetSysGetData()
{
    T object{};
    size_t netSysSize = sizeof(object);
    if (g_baseFuzzData == nullptr || netSysSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, netSysSize, g_baseFuzzData + g_baseFuzzPos, netSysSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += netSysSize;
    return object;
}

std::string NetSysGetString(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = NetSysGetData<char>();
    }
    std::string str(cstr);
    return str;
}

static bool g_isInited = false;

__attribute__((no_sanitize("cfi"))) void Init()
{
    nmd::IptablesWrapper::GetInstance();
    if (!DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->Init()) {
        g_isInited = false;
    } else {
        g_isInited = true;
    }
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
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

class NetDnsResultCallbackFuzzTest : public NetsysNative::NetDnsResultCallbackStub {
public:
    NetDnsResultCallbackFuzzTest() = default;
    ~NetDnsResultCallbackFuzzTest() override{};

    int32_t OnDnsResultReport(uint32_t size, const std::list<NetsysNative::NetDnsResultReport>) override
    {
        return 0;
    }
};

class TestNetDnsHealthCallbackFuzzTest : public NetsysNative::NetDnsHealthCallbackStub {
public:
    TestNetDnsHealthCallbackFuzzTest() = default;
    ~TestNetDnsHealthCallbackFuzzTest() override{};

    int32_t OnDnsHealthReport(const NetsysNative::NetDnsHealthReport &dnsHealthReport) override
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

__attribute__((no_sanitize("cfi"))) void NetworkCreatePhysicalFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = NetSysGetData<int32_t>();
    int32_t permission = NetSysGetData<int32_t>();

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

    int32_t netId = NetSysGetData<int32_t>();
    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_DESTROY), dataParcel);
}

void NetworkAddInterfaceFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = NetSysGetData<int32_t>();
    std::string iface = NetSysGetString(STR_LEN);

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

    int32_t netId = NetSysGetData<int32_t>();
    std::string iface = NetSysGetString(STR_LEN);

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

    int32_t netId = NetSysGetData<int32_t>();
    std::string ifName = NetSysGetString(STR_LEN);
    std::string destination = NetSysGetString(STR_LEN);
    std::string nextHop = NetSysGetString(STR_LEN);

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

    int32_t netId = NetSysGetData<int32_t>();
    std::string ifName = NetSysGetString(STR_LEN);
    std::string destination = NetSysGetString(STR_LEN);
    std::string nextHop = NetSysGetString(STR_LEN);

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
    cfg.ifName = NetSysGetString(STR_LEN);

    dataParcel.WriteString(cfg.ifName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_GET_CONFIG), dataParcel);
}

void GetInterfaceMtuFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string interfaceName = NetSysGetString(STR_LEN);

    dataParcel.WriteString(interfaceName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_GET_MTU), dataParcel);
}

void SetInterfaceMtuFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t mtu = NetSysGetData<int32_t>();
    std::string interfaceName = NetSysGetString(STR_LEN);

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

    std::string interfaceName = NetSysGetString(STR_LEN);
    std::string ipAddr = NetSysGetString(STR_LEN);
    int32_t prefixLength = NetSysGetData<int32_t>();

    dataParcel.WriteString(interfaceName);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteInt32(prefixLength);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_INTERFACE_ADD_ADDRESS), dataParcel);
}

int32_t OnRemoteRequestCallBack(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    NetsysNative::NotifyCallbackTest notifyCallBackTest;
    int32_t ret = notifyCallBackTest.OnRemoteRequest(code, data, reply, option);
    return ret;
}

void OnInterfaceAddressUpdatedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string addr = NetSysGetString(STR_LEN);
    std::string ifName = NetSysGetString(STR_LEN);
    int32_t flags = NetSysGetData<int32_t>();
    int32_t scope = NetSysGetData<int32_t>();

    dataParcel.WriteString(addr);
    dataParcel.WriteString(ifName);
    dataParcel.WriteInt32(flags);
    dataParcel.WriteInt32(scope);
    OnRemoteRequestCallBack(static_cast<uint32_t>(NetsysNative::NotifyInterfaceCode::ON_INTERFACE_ADDRESS_UPDATED),
                            dataParcel);
}

void RegisterNotifyCallbackFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    sptr<NetsysNative::NotifyCallbackStub> notifyCb = new (std::nothrow) NetsysNative::NotifyCallbackTest();
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

    sptr<NetsysNative::NotifyCallbackStub> notifyCb = new (std::nothrow) NetsysNative::NotifyCallbackTest();

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

    std::string ifaceName = NetSysGetString(STR_LEN);

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

    std::string hostName = NetSysGetString(STR_LEN);
    std::string serverName = NetSysGetString(STR_LEN);
    AddrInfo hints;
    hints.aiFlags = NetSysGetData<uint32_t>();
    hints.aiFamily = NetSysGetData<uint32_t>();
    hints.aiSockType = NetSysGetData<uint32_t>();
    hints.aiProtocol = NetSysGetData<uint32_t>();
    hints.aiAddrLen = NetSysGetData<uint32_t>();

    std::string aiCanName = NetSysGetString(STR_LEN);
    if (memcpy_s(hints.aiCanonName, sizeof(hints.aiCanonName), aiCanName.c_str(), aiCanName.length()) != 0) {
        return;
    }
    uint16_t netId = NetSysGetData<uint16_t>();

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

    int32_t netId = NetSysGetData<int32_t>();
    NetsysNative::RouteInfoParcel routInfo;
    routInfo.destination = NetSysGetString(STR_LEN);
    routInfo.ifName = NetSysGetString(STR_LEN);
    routInfo.nextHop = NetSysGetString(STR_LEN);
    routInfo.mtu = NetSysGetData<int32_t>();

    dataParcel.WriteInt32(netId);
    dataParcel.WriteString(routInfo.destination);
    dataParcel.WriteString(routInfo.ifName);
    dataParcel.WriteString(routInfo.nextHop);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_ADD_ROUTE_PARCEL),
                    dataParcel);
}

void NetworkRemoveRouteParcelFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = NetSysGetData<int32_t>();
    NetsysNative::RouteInfoParcel routInfo;
    routInfo.destination = NetSysGetString(STR_LEN);
    routInfo.ifName = NetSysGetString(STR_LEN);
    routInfo.nextHop = NetSysGetString(STR_LEN);

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

    int32_t netId = NetSysGetData<int32_t>();

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

    int32_t netId = NetSysGetData<int32_t>();

    dataParcel.WriteInt32(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NETWORK_SET_DEFAULT), dataParcel);
}

void IpfwdAddInterfaceForwardFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string fromIface = NetSysGetString(STR_LEN);
    std::string toIface = NetSysGetString(STR_LEN);

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

    std::string fromIface = NetSysGetString(STR_LEN);
    std::string toIface = NetSysGetString(STR_LEN);

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

    std::string ifaceName = NetSysGetString(STR_LEN);
    std::string ipAddress = NetSysGetString(STR_LEN);

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

    auto chain = NetSysGetData<uint32_t>();
    auto uidSize = static_cast<uint32_t>(NetSysGetData<uint8_t>());

    dataParcel.WriteUint32(chain);
    dataParcel.WriteUint32(uidSize);
    for (uint32_t index = 0; index < uidSize; index++) {
        dataParcel.WriteUint32(NetSysGetData<uint32_t>());
    }
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

    auto chain = NetSysGetData<uint32_t>();
    auto uidSize = static_cast<uint32_t>(NetSysGetData<uint8_t>());

    dataParcel.WriteUint32(chain);
    dataParcel.WriteUint32(uidSize);
    for (uint32_t index = 0; index < uidSize; index++) {
        dataParcel.WriteUint32(NetSysGetData<uint32_t>());
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_FIREWALL_SET_UID_DENIED_LIST_CHAIN),
                    dataParcel);
}

void FirewallSetUidRuleFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t chain = NetSysGetData<int32_t>();
    int32_t firewallRule = NetSysGetData<int32_t>();

    dataParcel.WriteInt32(chain);
    uint32_t vectorLength = NetSysGetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength));
    for (uint32_t i = 0; i <= vectorLength; i++) {
        dataParcel.WriteInt32(NetSysGetData<uint32_t>());
    }

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
    cfg.ifName = NetSysGetString(STR_LEN);
    cfg.hwAddr = NetSysGetString(STR_LEN);
    cfg.ipv4Addr = NetSysGetString(STR_LEN);
    cfg.prefixLength = NetSysGetData<int32_t>();

    dataParcel.WriteString(cfg.ifName);
    dataParcel.WriteString(cfg.hwAddr);
    dataParcel.WriteString(cfg.ipv4Addr);
    dataParcel.WriteInt32(cfg.prefixLength);
    uint32_t vectorLength = NetSysGetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength));
    for (uint32_t i = 0; i <= vectorLength; i++) {
        dataParcel.WriteString(NetSysGetString(STR_LEN));
    }

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

    int32_t family = NetSysGetData<int32_t>();
    int32_t which = NetSysGetData<int32_t>();
    std::string ifname = NetSysGetString(STR_LEN);
    std::string parameter = NetSysGetString(STR_LEN);

    dataParcel.WriteInt32(family);
    dataParcel.WriteInt32(which);
    dataParcel.WriteString(ifname);
    dataParcel.WriteString(parameter);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_PROC_SYS_NET), dataParcel);
}

void SetProcSysNetFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t family = NetSysGetData<int32_t>();
    int32_t which = NetSysGetData<int32_t>();
    std::string ifname = NetSysGetString(STR_LEN);
    std::string parameter = NetSysGetString(STR_LEN);
    std::string value = NetSysGetString(STR_LEN);

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

    uint32_t uid = NetSysGetData<uint32_t>();
    int8_t allow = NetSysGetData<int8_t>();

    dataParcel.WriteUint32(uid);
    dataParcel.WriteInt8(allow);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_INTERNET_PERMISSION),
                    dataParcel);
}

void GetFwmarkForNetworkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    int32_t netId = NetSysGetData<int32_t>();
    NetsysNative::MarkMaskParcel markParcl;
    markParcl.mark = NetSysGetData<int32_t>();
    markParcl.mask = NetSysGetData<int32_t>();

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

    std::string requestor = NetSysGetString(STR_LEN);

    dataParcel.WriteString(requestor);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_IPENABLE_FORWARDING), dataParcel);
}

void IpDisableForwardingFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string requestor = NetSysGetString(STR_LEN);

    dataParcel.WriteString(requestor);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_IPDISABLE_FORWARDING), dataParcel);
}

void EnableNatFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string downstreamIface = NetSysGetString(STR_LEN);
    std::string upstreamIface = NetSysGetString(STR_LEN);

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

    std::string downstreamIface = NetSysGetString(STR_LEN);
    std::string upstreamIface = NetSysGetString(STR_LEN);

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

    bool enable = NetSysGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

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

    int64_t bytes = NetSysGetData<int64_t>();
    std::string ifName = NetSysGetString(STR_LEN);

    dataParcel.WriteString(ifName);
    dataParcel.WriteInt64(bytes);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_BANDWIDTH_SET_IFACE_QUOTA),
                    dataParcel);
}

void BandwidthRemoveIfaceQuotaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string ifName = NetSysGetString(STR_LEN);

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

    uint32_t uid = NetSysGetData<uint32_t>();

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

    uint32_t uid = NetSysGetData<uint32_t>();

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

    uint32_t uid = NetSysGetData<uint32_t>();

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

    uint32_t uid = NetSysGetData<uint32_t>();

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

    uint32_t chain = NetSysGetData<uint32_t>();
    bool enable = NetSysGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

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

    std::string downIface = NetSysGetString(STR_LEN);
    std::string upIface = NetSysGetString(STR_LEN);

    dataParcel.WriteString(downIface);
    dataParcel.WriteString(upIface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_SHARING_NETWORK_TRAFFIC),
                    dataParcel);
}

void DelInterfaceAddressFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    std::string interfaceName = NetSysGetString(STR_LEN);
    std::string ipAddr = NetSysGetString(STR_LEN);
    int32_t prefixLength = NetSysGetData<int32_t>();

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

    uint16_t netId = NetSysGetData<uint16_t>();
    uint16_t baseTimeoutMsec = NetSysGetData<uint16_t>();
    uint8_t retryCount = NetSysGetData<uint8_t>();

    dataParcel.WriteUint16(netId);
    dataParcel.WriteUint16(baseTimeoutMsec);
    dataParcel.WriteUint8(retryCount);

    uint32_t vectorLength = NetSysGetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength));
    for (uint32_t i = 0; i <= vectorLength; i++) {
        dataParcel.WriteString(NetSysGetString(STR_LEN));
    }

    uint32_t vectorLength2 = NetSysGetData<uint32_t>() % VECTOR_MAX_SIZE;
    dataParcel.WriteInt32(static_cast<int32_t>(vectorLength2));
    for (uint32_t i = 0; i <= vectorLength2; i++) {
        dataParcel.WriteString(NetSysGetString(STR_LEN));
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_RESOLVER_CONFIG), dataParcel);
}

void GetResolverConfigFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = NetSysGetData<uint16_t>();

    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_RESOLVER_CONFIG), dataParcel);
}

void CreateNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = NetSysGetData<uint16_t>();

    dataParcel.WriteUint16(netId);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_CREATE_NETWORK_CACHE), dataParcel);
}

void DestroyNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint16_t netId = NetSysGetData<uint16_t>();

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
    uint16_t netId = NetSysGetData<uint16_t>();
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
    std::string iface = NetSysGetString(STR_LEN);
    bool bIpv6 = NetSysGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
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
    std::string iface = NetSysGetString(STR_LEN);
    bool bIpv6 = NetSysGetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;

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
    std::string iface = NetSysGetString(STR_LEN);
    std::string ipv4addr = NetSysGetString(STR_LEN);
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
    std::string iface = NetSysGetString(STR_LEN);
    dataParcel.WriteString(iface);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_STOP_DHCP_SERVICE), dataParcel);
}

void GetTotalStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t type = NetSysGetData<uint32_t>();
    dataParcel.WriteUint32(type);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_TOTAL_STATS), dataParcel);
}

void GetUidStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint32_t type = NetSysGetData<uint32_t>();
    uint32_t uid = NetSysGetData<uint32_t>();

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
    uint32_t type = NetSysGetData<uint32_t>();
    std::string iface = NetSysGetString(STR_LEN);

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

    int32_t netId = NetSysGetData<int32_t>();
    bool hasDns = NetSysGetData<bool>();

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

    int32_t netId = NetSysGetData<int32_t>();
    std::vector<UidRange> uidRanges;
    UidRange uid;
    int32_t rangesSize = NetSysGetData<int32_t>() % VECTOR_MAX_SIZE;
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

    int32_t netId = NetSysGetData<int32_t>();
    std::vector<UidRange> uidRanges;
    UidRange uid;
    int32_t rangesSize = NetSysGetData<int32_t>() % VECTOR_MAX_SIZE;
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

void GetCookieStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint32_t type = NetSysGetData<uint32_t>();
    uint64_t cookie = NetSysGetData<uint64_t>();

    dataParcel.WriteUint32(type);
    dataParcel.WriteUint64(cookie);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_COOKIE_STATS), dataParcel);
}

void CmdCreateNetworkCacheFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint16_t netId = NetSysGetData<uint16_t>();
    dataParcel.WriteUint16(netId);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_CREATE_NETWORK_CACHE), dataParcel);
}

void CmdGetTotalStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint8_t type = NetSysGetData<uint8_t>();
    dataParcel.WriteUint8(type);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_TOTAL_STATS), dataParcel);
}

void CmdSetTcpBufferSizesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string tcpBufferSizes = NetSysGetString(STR_LEN);
    dataParcel.WriteString(tcpBufferSizes);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_TCP_BUFFER_SIZES), dataParcel);
}

void CmdGetAllStatsInfoFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_ALL_STATS_INFO), dataParcel);
}

void CmdSetIptablesCommandForResFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string cmd = NetSysGetString(STR_LEN);
    dataParcel.WriteString(cmd);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_IPTABLES_CMD_FOR_RES),
                    dataParcel);
}

void CmdAddStaticArpFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string ipAddr = NetSysGetString(STR_LEN);
    dataParcel.WriteString(ipAddr);
    std::string macAddr = NetSysGetString(STR_LEN);
    dataParcel.WriteString(macAddr);
    std::string ifName = NetSysGetString(STR_LEN);
    dataParcel.WriteString(ifName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_ADD_STATIC_ARP), dataParcel);
}

void CmdDelStaticArpFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    std::string ifName = NetSysGetString(STR_LEN);
    std::string macAddr = NetSysGetString(STR_LEN);
    std::string ipAddr = NetSysGetString(STR_LEN);
    dataParcel.WriteString(ipAddr);
    dataParcel.WriteString(macAddr);
    dataParcel.WriteString(ifName);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_DEL_STATIC_ARP), dataParcel);
}

void CmdRegisterDnsResultListenerFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    sptr<NetsysNative::INetDnsResultCallback> callback = new (std::nothrow) NetDnsResultCallbackFuzzTest();
    if (!dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    uint32_t timeStep = NetSysGetData<uint32_t>();
    dataParcel.WriteUint32(timeStep);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_REGISTER_DNS_RESULT_LISTENER),
                    dataParcel);
}

void CmdUnregisterDnsResultListenerFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    sptr<NetsysNative::INetDnsResultCallback> callback = new (std::nothrow) NetDnsResultCallbackFuzzTest();
    if (!dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_UNREGISTER_DNS_RESULT_LISTENER),
                    dataParcel);
}

void CmdRegisterDnsHealthListenerFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    sptr<NetsysNative::INetDnsHealthCallback> callback = new (std::nothrow) TestNetDnsHealthCallbackFuzzTest();
    if (!dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_REGISTER_DNS_HEALTH_LISTENER),
                    dataParcel);
}

void CmdUnregisterDnsHealthListenerFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    sptr<NetsysNative::INetDnsHealthCallback> callback = new (std::nothrow) TestNetDnsHealthCallbackFuzzTest();
    if (!dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_UNREGISTER_DNS_HEALTH_LISTENER),
                    dataParcel);
}

void CmdGetNetworkSharingTypeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_GET_NETWORK_SHARING_TYPE),
                    dataParcel);
}

void CmdUpdateNetworkSharingTypeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    uint32_t type = NetSysGetData<uint32_t>();
    bool isOpen = NetSysGetData<bool>();
    dataParcel.WriteUint32(type);
    dataParcel.WriteBool(isOpen);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_UPDATE_NETWORK_SHARING_TYPE),
                    dataParcel);
}

void CmdSetNetworkAccessPolicyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }
    NetworkAccessPolicy netAccessPolicy;
    uint32_t uid = NetSysGetData<uint32_t>();
    netAccessPolicy.wifiAllow = NetSysGetData<bool>();
    netAccessPolicy.cellularAllow = NetSysGetData<bool>();
    bool reconfirmFlag = NetSysGetData<bool>();

    dataParcel.WriteUint32(uid);
    dataParcel.WriteUint8(netAccessPolicy.wifiAllow);
    dataParcel.WriteUint8(netAccessPolicy.cellularAllow);
    dataParcel.WriteBool(reconfirmFlag);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_NETWORK_ACCESS_POLICY),
                    dataParcel);
}

void CmdDeleteNetworkAccessPolicyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t uid = NetSysGetData<uint32_t>();
    dataParcel.WriteUint32(uid);
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_DEL_NETWORK_ACCESS_POLICY),
                    dataParcel);
}

void CmdNotifyNetBearerTypeChangeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    uint32_t rangesSize = NetSysGetData<uint32_t>();
    uint32_t bearerType = NetSysGetData<uint32_t>();

    std::set<uint32_t> bearerTypes;
    dataParcel.WriteUint32(rangesSize);
    for (uint32_t i = 0; i < rangesSize; i++) {
        bearerTypes.insert(static_cast<uint32_t>(bearerType));
    }

    for (auto iter : bearerTypes) {
        dataParcel.WriteUint32(iter);
    }

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_NOTIFY_NETWORK_BEARER_TYPE_CHANGE),
                    dataParcel);
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
    OHOS::NetManagerStandard::GetCookieStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdCreateNetworkCacheFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdGetTotalStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdSetTcpBufferSizesFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdGetAllStatsInfoFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdSetIptablesCommandForResFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdAddStaticArpFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdDelStaticArpFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdRegisterDnsResultListenerFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdUnregisterDnsResultListenerFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdRegisterDnsHealthListenerFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdUnregisterDnsHealthListenerFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdGetNetworkSharingTypeFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdUpdateNetworkSharingTypeFuzzTest(data, size);
}

void LLVMFuzzerTestOneInputOthers(const uint8_t *data, size_t size)
{
    OHOS::NetManagerStandard::CmdSetNetworkAccessPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdDeleteNetworkAccessPolicyFuzzTest(data, size);
    OHOS::NetManagerStandard::CmdNotifyNetBearerTypeChangeFuzzTest(data, size);
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
    OHOS::NetManagerStandard::LLVMFuzzerTestOneInputOthers(data, size);
    return 0;
}