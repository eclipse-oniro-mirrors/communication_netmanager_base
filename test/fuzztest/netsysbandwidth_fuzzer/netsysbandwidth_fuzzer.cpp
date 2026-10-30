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

#include "netsysbandwidth_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void BandwidthEnableDataSaverCmd(MessageParcel &parcel)
{
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // enable
}

void BandwidthSetIfaceQuotaCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ifName
    parcel.WriteInt64(GetData<int64_t>()); // bytes
}

void BandwidthRemoveIfaceQuotaCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ifName
}

void BandwidthAddDeniedListCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // uid
}

void BandwidthRemoveDeniedListCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // uid
}

void BandwidthAddAllowedListCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // uid
}

void BandwidthRemoveAllowedListCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // uid
}

void GetSharingNetworkTrafficCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // downIface
    parcel.WriteString(GetStringFromData()); // upIface
}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 8;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_BANDWIDTH_ENABLE_DATA_SAVER, BandwidthEnableDataSaverCmd},
    {Code::NETSYS_BANDWIDTH_SET_IFACE_QUOTA, BandwidthSetIfaceQuotaCmd},
    {Code::NETSYS_BANDWIDTH_REMOVE_IFACE_QUOTA, BandwidthRemoveIfaceQuotaCmd},
    {Code::NETSYS_BANDWIDTH_ADD_DENIED_LIST, BandwidthAddDeniedListCmd},
    {Code::NETSYS_BANDWIDTH_REMOVE_DENIED_LIST, BandwidthRemoveDeniedListCmd},
    {Code::NETSYS_BANDWIDTH_ADD_ALLOWED_LIST, BandwidthAddAllowedListCmd},
    {Code::NETSYS_BANDWIDTH_REMOVE_ALLOWED_LIST, BandwidthRemoveAllowedListCmd},
    {Code::NETSYS_GET_SHARING_NETWORK_TRAFFIC, GetSharingNetworkTrafficCmd},
};
} // namespace

void NetsysBandwidthFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysBandwidthFuzzTest(data, size);
    return 0;
}
