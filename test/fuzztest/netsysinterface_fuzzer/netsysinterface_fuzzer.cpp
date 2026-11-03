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

#include "netsysinterface_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void InterfaceSetMtuCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData());
    parcel.WriteInt32(GetData<int32_t>());
}

void InterfaceGetMtuCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData());
}

void InterfaceAddAddressCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData());
    parcel.WriteString(GetStringFromData());
    parcel.WriteInt32(GetData<int32_t>());
}

void InterfaceDelAddressCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData());
    parcel.WriteString(GetStringFromData());
    parcel.WriteInt32(GetData<int32_t>());
}

void InterfaceSetIffUpCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData());
}

void InterfaceGetListCmd(MessageParcel &parcel) {}

void InterfaceSetConfigCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteString(GetStringFromData()); // hwAddr
    parcel.WriteString(GetStringFromData()); // ipv4Addr
    parcel.WriteInt32(GetData<int32_t>()); // prefixLength
    int32_t size = static_cast<int32_t>(GetData<uint32_t>() % MAX_VECTOR_LEN);
    parcel.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        parcel.WriteString(GetStringFromData()); // flags
    }
}

void InterfaceGetConfigCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData());
}

void GetIpNeighTableCmd(MessageParcel &parcel) {}

void SetIpv6AutoConfCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // interfaceName
    parcel.WriteInt32(GetData<int32_t>()); // on
}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 10;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_INTERFACE_SET_MTU, InterfaceSetMtuCmd},
    {Code::NETSYS_INTERFACE_GET_MTU, InterfaceGetMtuCmd},
    {Code::NETSYS_INTERFACE_ADD_ADDRESS, InterfaceAddAddressCmd},
    {Code::NETSYS_INTERFACE_DEL_ADDRESS, InterfaceDelAddressCmd},
    {Code::NETSYS_INTERFACE_SET_IFF_UP, InterfaceSetIffUpCmd},
    {Code::NETSYS_INTERFACE_GET_LIST, InterfaceGetListCmd},
    {Code::NETSYS_INTERFACE_SET_CONFIG, InterfaceSetConfigCmd},
    {Code::NETSYS_INTERFACE_GET_CONFIG, InterfaceGetConfigCmd},
    {Code::NETSYS_GET_IP_NEIGH_TABLE, GetIpNeighTableCmd},
    {Code::NETSYS_NETWORK_SET_IPV6_AUTO_CONF, SetIpv6AutoConfCmd},
};
} // namespace

void NetsysInterfaceFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysInterfaceFuzzTest(data, size);
    return 0;
}
