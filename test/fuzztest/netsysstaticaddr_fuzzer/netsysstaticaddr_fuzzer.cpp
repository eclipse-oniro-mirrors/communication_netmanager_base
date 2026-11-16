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

#include "netsysstaticaddr_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "net_conn_info.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void AddStaticIpv6Cmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ipAddr
    parcel.WriteString(GetStringFromData()); // macAddr
    parcel.WriteString(GetStringFromData()); // ifName
}

void DelStaticIpv6Cmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ipAddr
    parcel.WriteString(GetStringFromData()); // macAddr
    parcel.WriteString(GetStringFromData()); // ifName
}

void AddStaticArpCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ipAddr
    parcel.WriteString(GetStringFromData()); // macAddr
    parcel.WriteString(GetStringFromData()); // ifName
}

void DelStaticArpCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ipAddr
    parcel.WriteString(GetStringFromData()); // macAddr
    parcel.WriteString(GetStringFromData()); // ifName
}

void SetInternetAccessByIpForWifiShareCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // ipAddr
    parcel.WriteUint8(GetData<uint8_t>()); // family
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // accessInternet
    parcel.WriteString(GetStringFromData()); // clientNetIfName
}

void GetConnectOwnerUidCmd(MessageParcel &parcel)
{
    NetConnInfo netConnInfo;
    netConnInfo.protocolType_ = GetData<int32_t>();
    netConnInfo.family_ = static_cast<NetConnInfo::Family>(GetData<uint32_t>());
    netConnInfo.localAddress_ = GetStringFromData();
    netConnInfo.localPort_ = GetData<uint16_t>();
    netConnInfo.remoteAddress_ = GetStringFromData();
    netConnInfo.remotePort_ = GetData<uint16_t>();
    netConnInfo.Marshalling(parcel);
}

#ifdef SUPPORT_SYSVPN
void UpdateVpnRulesCmd(MessageParcel &parcel)
{
    parcel.WriteUint16(GetData<uint16_t>()); // netId
    int32_t size = static_cast<int32_t>(GetData<uint32_t>() % MAX_VECTOR_LEN);
    parcel.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        parcel.WriteString(GetStringFromData()); // extMessages
    }
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // add
}
#endif

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

#ifdef SUPPORT_SYSVPN
constexpr uint32_t TARGET_SIZE = 7;
#else
constexpr uint32_t TARGET_SIZE = 6;
#endif
const CmdEntry TARGETS[] = {
    {Code::NETSYS_ADD_STATIC_IPV6, AddStaticIpv6Cmd},
    {Code::NETSYS_DEL_STATIC_IPV6, DelStaticIpv6Cmd},
    {Code::NETSYS_ADD_STATIC_ARP, AddStaticArpCmd},
    {Code::NETSYS_DEL_STATIC_ARP, DelStaticArpCmd},
    {Code::NETSYS_SET_INTERNET_ACCESS_BY_IP_FOR_NET_SHARE, SetInternetAccessByIpForWifiShareCmd},
    {Code::NETSYS_GET_CONNECT_OWNER_UID, GetConnectOwnerUidCmd},
#ifdef SUPPORT_SYSVPN
    {Code::NETSYS_UPDATE_VPN_RULES, UpdateVpnRulesCmd},
#endif
};
static_assert(sizeof(TARGETS) / sizeof(TARGETS[0]) == TARGET_SIZE, "target table size mismatch");
} // namespace

void NetsysStaticAddrFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysStaticAddrFuzzTest(data, size);
    return 0;
}
