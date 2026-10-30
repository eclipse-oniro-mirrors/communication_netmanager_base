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

#include "netsysmisc_fuzzer.h"

#include "common_netsys_fuzzer_utils.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
namespace {
using Code = NetsysInterfaceCode;

void StartDhcpClientCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // iface
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // bIpv6
}

void StopDhcpClientCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // iface
    parcel.WriteBool(GetData<uint32_t>() % CONVERT_NUMBER_TO_BOOL == 0); // bIpv6
}

void StartDhcpServiceCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // iface
    parcel.WriteString(GetStringFromData()); // ipv4addr
}

void StopDhcpServiceCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // iface
}

void EnableNatCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // downstreamIface
    parcel.WriteString(GetStringFromData()); // upstreamIface
}

void DisableNatCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // downstreamIface
    parcel.WriteString(GetStringFromData()); // upstreamIface
}

void IpfwdAddInterfaceForwardCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // fromIface
    parcel.WriteString(GetStringFromData()); // toIface
}

void IpfwdRemoveInterfaceForwardCmd(MessageParcel &parcel)
{
    parcel.WriteString(GetStringFromData()); // fromIface
    parcel.WriteString(GetStringFromData()); // toIface
}

void GetProcSysNetCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>()); // family
    parcel.WriteInt32(GetData<int32_t>()); // which
    parcel.WriteString(GetStringFromData()); // ifname
    parcel.WriteString(GetStringFromData()); // parameter
}

void SetProcSysNetCmd(MessageParcel &parcel)
{
    parcel.WriteInt32(GetData<int32_t>()); // family
    parcel.WriteInt32(GetData<int32_t>()); // which
    parcel.WriteString(GetStringFromData()); // ifname
    parcel.WriteString(GetStringFromData()); // parameter
    parcel.WriteString(GetStringFromData()); // value
}

struct CmdEntry {
    Code code;
    void (*writer)(MessageParcel &);
};

constexpr uint32_t TARGET_SIZE = 10;
const CmdEntry TARGETS[TARGET_SIZE] = {
    {Code::NETSYS_START_DHCP_CLIENT, StartDhcpClientCmd},
    {Code::NETSYS_STOP_DHCP_CLIENT, StopDhcpClientCmd},
    {Code::NETSYS_START_DHCP_SERVICE, StartDhcpServiceCmd},
    {Code::NETSYS_STOP_DHCP_SERVICE, StopDhcpServiceCmd},
    {Code::NETSYS_ENABLE_NAT, EnableNatCmd},
    {Code::NETSYS_DISABLE_NAT, DisableNatCmd},
    {Code::NETSYS_IPFWD_ADD_INTERFACE_FORWARD, IpfwdAddInterfaceForwardCmd},
    {Code::NETSYS_IPFWD_REMOVE_INTERFACE_FORWARD, IpfwdRemoveInterfaceForwardCmd},
    {Code::NETSYS_GET_PROC_SYS_NET, GetProcSysNetCmd},
    {Code::NETSYS_SET_PROC_SYS_NET, SetProcSysNetCmd},
};
} // namespace

void NetsysMiscFuzzTest(const uint8_t *data, size_t size)
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
    OHOS::NetsysNative::NetsysFuzzer::NetsysMiscFuzzTest(data, size);
    return 0;
}
