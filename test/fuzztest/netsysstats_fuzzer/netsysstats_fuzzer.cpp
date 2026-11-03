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

#include "netsysstats_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void GetTotalStatsCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // type
}

void GetUidStatsCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // type
    parcel.WriteUint32(GetData<uint32_t>()); // uid
}

void GetIfaceStatsCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // type
    parcel.WriteString(GetStringFromData()); // iface
}

void GetAllStatsInfoCmd(MessageParcel &parcel) {}

void GetCookieStatsCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // type
    parcel.WriteUint64(GetData<uint64_t>()); // cookie
}

void DeleteStatsInfoCmd(MessageParcel &parcel)
{
    parcel.WriteUint32(GetData<uint32_t>()); // uid
}

void DeleteIncreaseTrafficMapCmd(MessageParcel &parcel)
{
    parcel.WriteUint64(GetData<uint64_t>()); // ifIndex
}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 7;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_GET_TOTAL_STATS, GetTotalStatsCmd},
    {Code::NETSYS_GET_UID_STATS, GetUidStatsCmd},
    {Code::NETSYS_GET_IFACE_STATS, GetIfaceStatsCmd},
    {Code::NETSYS_GET_ALL_STATS_INFO, GetAllStatsInfoCmd},
    {Code::NETSYS_GET_COOKIE_STATS, GetCookieStatsCmd},
    {Code::NETSYS_DELETE_STATS_INFO, DeleteStatsInfoCmd},
    {Code::NETSYS_DELETE_INCRE_TRAFFIC_MAP, DeleteIncreaseTrafficMapCmd},
};
} // namespace

void NetsysStatsFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysStatsFuzzTest(data, size);
    return 0;
}
