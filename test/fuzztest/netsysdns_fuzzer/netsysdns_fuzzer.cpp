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

#include "netsysdns_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void SetResolverConfigCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // netId
    parcel.WriteUint16(GetData<uint16_t>()); // baseTimeoutMsec
    parcel.WriteUint8(GetData<uint8_t>()); // retryCount
    int32_t serverSize = static_cast<int32_t>(GetData<uint32_t>() % MAX_VECTOR_LEN);
    parcel.WriteInt32(serverSize);
    for (int32_t i = 0; i < serverSize; i++) {
        parcel.WriteString(GetStringFromData()); // servers
    }
    int32_t domainSize = static_cast<int32_t>(GetData<uint32_t>() % MAX_VECTOR_LEN);
    parcel.WriteInt32(domainSize);
    for (int32_t i = 0; i < domainSize; i++) {
        parcel.WriteString(GetStringFromData()); // domains
    }
}

void GetResolverConfigCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // netId
}

void CreateNetworkCacheCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // netId
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // isVpnNet
}

void DestroyNetworkCacheCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // netId
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // isVpnNet
}

void FlushDnsCacheCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // netId
}

void GetAddrInfoCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // hostName
    parcel.WriteString(GetStringFromData()); // serverName
    AddrInfo hints = {};
    hints.aiFlags = GetData<uint32_t>();
    hints.aiFamily = GetData<uint32_t>();
    hints.aiSockType = GetData<uint32_t>();
    hints.aiProtocol = GetData<uint32_t>();
    parcel.WriteRawData(&hints, sizeof(AddrInfo)); // hints
    parcel.WriteUint16(GetData<uint16_t>()); // netId
}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 6;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_SET_RESOLVER_CONFIG, SetResolverConfigCmd},
    {Code::NETSYS_GET_RESOLVER_CONFIG, GetResolverConfigCmd},
    {Code::NETSYS_CREATE_NETWORK_CACHE, CreateNetworkCacheCmd},
    {Code::NETSYS_DESTROY_NETWORK_CACHE, DestroyNetworkCacheCmd},
    {Code::NETSYS_FLUSH_DNS_CACHE, FlushDnsCacheCmd},
    {Code::NETSYS_GET_ADDR_INFO, GetAddrInfoCmd},
};
} // namespace

void NetsysDnsFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysDnsFuzzTest(data, size);
    return 0;
}
