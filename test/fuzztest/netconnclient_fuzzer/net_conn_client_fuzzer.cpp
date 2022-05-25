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
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "net_conn_client.h"
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
class INetConnCallbackTest : public INetConnCallback {
public:
    INetConnCallbackTest() : INetConnCallback() {}
    virtual ~INetConnCallbackTest() {}
};

class NetSupplierCallbackBaseTest : public NetSupplierCallbackBase {
public:
    NetSupplierCallbackBaseTest() : NetSupplierCallbackBase() {}
    virtual ~NetSupplierCallbackBaseTest() {}
};

void RegisterNetSupplierFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string ident(reinterpret_cast<const char*>(data), size);
    std::set<NetCap> netCaps {NET_CAPABILITY_INTERNET, NET_CAPABILITY_MMS};
    NetBearType bearerType = BEARER_CELLULAR;
    uint32_t supplierId = *(reinterpret_cast<const uint32_t*>(data));
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
}

void UnregisterNetSupplierFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    uint32_t supplierId = *(reinterpret_cast<const uint32_t*>(data));
    DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(supplierId);
}

void HasDefaultNetFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    bool flag = *(reinterpret_cast<const bool*>(data));
    DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(flag);
}

void GetAllNetsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::list<sptr<NetHandle>> netList;
    DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(netList);
}

void BindSocketFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    int32_t socket_fd = *(reinterpret_cast<const int32_t*>(data));
    int32_t netId = *(reinterpret_cast<const int32_t*>(data));

    DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socket_fd, netId);
}

void SetAirplaneModeFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    bool state = *(reinterpret_cast<const bool*>(data));

    DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(state);
}

void UpdateNetSupplierInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    uint32_t supplierId = *(reinterpret_cast<const uint32_t*>(data));
    sptr<NetSupplierInfo> netSupplierInfo;

    DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
}

void GetAddressByNameFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string host(reinterpret_cast<const char*>(data), size);
    int32_t netId = *(reinterpret_cast<const int32_t*>(data));
    INetAddr addr;

    DelayedSingleton<NetConnClient>::GetInstance()->GetAddressByName(host, netId, addr);
}

void GetAddressesByNameFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string host(reinterpret_cast<const char*>(data), size);
    int32_t netId = *(reinterpret_cast<const int32_t*>(data));
    std::vector<INetAddr> addrList;

    DelayedSingleton<NetConnClient>::GetInstance()->GetAddressesByName(host, netId, addrList);
}

void UpdateNetLinkInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    uint32_t supplierId = *(reinterpret_cast<const uint32_t*>(data));
    sptr<NetLinkInfo> netLinkInfo;

    DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
}

void RegisterNetSupplierCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    uint32_t supplierId = *(reinterpret_cast<const uint32_t*>(data));
    sptr<NetSupplierCallbackBaseTest> callback = sptr<NetSupplierCallbackBaseTest>();

    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplierCallback(supplierId, callback);
}

void RegisterNetConnCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    sptr<NetSpecifier> netSpecifier;
    sptr<INetConnCallbackTest> callback = sptr<INetConnCallbackTest>();
    uint32_t timeoutMS = *(reinterpret_cast<const uint32_t*>(data));

    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback, timeoutMS);
    DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
}

void UnregisterNetConnCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    sptr<INetConnCallbackTest> callback = sptr<INetConnCallbackTest>();

    DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
}

void GetDefaultNetFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    NetHandle netHandle(static_cast<const int32_t>(*data));

    DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(netHandle);
}

void GetConnectionPropertiesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    NetLinkInfo info;
    NetHandle netHandle(static_cast<const int32_t>(*data));

    DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(netHandle, info);
}

void GetNetCapabilitiesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    NetAllCapabilities netAllCap;
    NetHandle netHandle(static_cast<const int32_t>(*data));

    DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(netHandle, netAllCap);
}

void NetDetectionFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    NetHandle netHandle(static_cast<const int32_t>(*data));

    DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(netHandle);
}
} // NetManagerStandard
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::RegisterNetSupplierFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetSupplierFuzzTest(data, size);
    OHOS::NetManagerStandard::HasDefaultNetFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAllNetsFuzzTest(data, size);
    OHOS::NetManagerStandard::BindSocketFuzzTest(data, size);
    OHOS::NetManagerStandard::SetAirplaneModeFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAddressByNameFuzzTest(data, size);
    OHOS::NetManagerStandard::GetAddressesByNameFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetSupplierCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetConnCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::NetDetectionFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetConnCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetDefaultNetFuzzTest(data, size);
    OHOS::NetManagerStandard::GetConnectionPropertiesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetNetCapabilitiesFuzzTest(data, size);

    return 0;
}
