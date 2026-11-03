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

#include "netsysvlan_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void CreateVlanCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteUint32(GetData<uint32_t>()); // vlanId
}

void DestroyVlanCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteUint32(GetData<uint32_t>()); // vlanId
}

void AddVlanIpCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteUint32(GetData<uint32_t>()); // vlanId
    parcel.WriteString(GetStringFromData()); // ip
    parcel.WriteUint32(GetData<uint32_t>()); // mask
}

void CreateVnicCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // mtu
    parcel.WriteString(GetStringFromData()); // tunAddr
    parcel.WriteInt32(GetData<int32_t>()); // prefix
    int32_t size = static_cast<int32_t>(GetData<uint32_t>() % MAX_VECTOR_LEN);
    parcel.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        parcel.WriteInt32(GetData<int32_t>()); // uid
    }
}

void DestroyVnicCmd(MessageParcel &parcel) {}

void GetSystemNetPortStatesCmd(MessageParcel &parcel) {}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 6;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_CREATE_VLAN, CreateVlanCmd},
    {Code::NETSYS_DESTROY_VLAN, DestroyVlanCmd},
    {Code::NETSYS_ADD_VLAN_IP, AddVlanIpCmd},
    {Code::NETSYS_VNIC_CREATE, CreateVnicCmd},
    {Code::NETSYS_VNIC_DESTROY, DestroyVnicCmd},
    {Code::NETSYS_GET_SYSTEM_NET_PORT_STATES, GetSystemNetPortStatesCmd},
};
} // namespace

void NetsysVlanFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysVlanFuzzTest(data, size);
    return 0;
}
