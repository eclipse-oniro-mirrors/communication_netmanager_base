/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "netsysnetwork_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"
#include "route_type.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void NetworkCreatePhysicalCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
    parcel.WriteInt32(GetData<int32_t>());
}

void NetworkDestroyCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
}

void NetworkAddInterfaceCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
    parcel.WriteString(GetStringFromData());
}

void NetworkRemoveInterfaceCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
    parcel.WriteString(GetStringFromData());
}

void NetworkAddRouteCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
    parcel.WriteString(GetStringFromData());
    parcel.WriteString(GetStringFromData());
    parcel.WriteString(GetStringFromData());
}

void NetworkAddRoutesCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>()); // netId
    int32_t size = static_cast<int32_t>(GetData<uint32_t>() % MAX_VECTOR_LEN);
    parcel.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        nmd::NetworkRouteInfo info;
        info.ifName = GetStringFromData();
        info.destination = GetStringFromData();
        info.nextHop = GetStringFromData();
        info.isExcludedRoute = GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0;
        info.Marshalling(parcel);
    }
}

void NetworkRemoveRouteCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
    parcel.WriteString(GetStringFromData());
    parcel.WriteString(GetStringFromData());
    parcel.WriteString(GetStringFromData());
}

void NetworkAddRouteParcelCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>()); // netId
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteString(GetStringFromData()); // destination
    parcel.WriteString(GetStringFromData()); // nextHop
}

void NetworkRemoveRouteParcelCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>()); // netId
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteString(GetStringFromData()); // destination
    parcel.WriteString(GetStringFromData()); // nextHop
}

void NetworkSetDefaultCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>());
}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 10;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_NETWORK_CREATE_PHYSICAL, NetworkCreatePhysicalCmd},
    {Code::NETSYS_NETWORK_DESTROY, NetworkDestroyCmd},
    {Code::NETSYS_NETWORK_ADD_INTERFACE, NetworkAddInterfaceCmd},
    {Code::NETSYS_NETWORK_REMOVE_INTERFACE, NetworkRemoveInterfaceCmd},
    {Code::NETSYS_NETWORK_ADD_ROUTE, NetworkAddRouteCmd},
    {Code::NETSYS_NETWORK_ADD_ROUTES, NetworkAddRoutesCmd},
    {Code::NETSYS_NETWORK_REMOVE_ROUTE, NetworkRemoveRouteCmd},
    {Code::NETSYS_NETWORK_ADD_ROUTE_PARCEL, NetworkAddRouteParcelCmd},
    {Code::NETSYS_NETWORK_REMOVE_ROUTE_PARCEL, NetworkRemoveRouteParcelCmd},
    {Code::NETSYS_NETWORK_SET_DEFAULT, NetworkSetDefaultCmd},
};
} // namespace

void NetsysNetworkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsValidFuzzData(data, size, parcel)) {
        return;
    }
    uint32_t index = GetData<uint8_t>() % TARGET_SIZE;
    TARGETS[index].writer(parcel);
    DispatchRequest(static_cast<uint32_t>(TARGETS[index].code), parcel);
}
} // namespace NetsysFuzzer
} // namespace NetsysNative
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::NetsysNative::NetsysFuzzer::NetsysNetworkFuzzTest(data, size);
    return 0;
}
