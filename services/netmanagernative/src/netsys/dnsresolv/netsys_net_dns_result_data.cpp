/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "netnative_log_wrapper.h"
#include "netsys_net_dns_result_data.h"

namespace OHOS {
namespace NetsysNative {

bool NetDnsResultAddrInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(type_)) {
        return false;
    }
    if (!parcel.WriteString(addr_)) {
        return false;
    }
    return true;
}


bool NetDnsResultAddrInfo::Unmarshalling(Parcel &parcel, NetDnsResultAddrInfo &addrInfo)
{
    if (!parcel.ReadUint32(addrInfo.type_)) {
        return false;
    }

    if (!parcel.ReadString(addrInfo.addr_)) {
        return false;
    }

    return true;
}

bool NetDnsResultReport::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(netid_)) {
        return false;
    }
    if (!parcel.WriteUint32(uid_)) {
        return false;
    }
    if (!parcel.WriteUint32(pid_)) {
        return false;
    }
    if (!parcel.WriteUint32(timeused_)) {
        return false;
    }
    if (!parcel.WriteUint32(queryresult_)) {
        return false;
    }
    if (!parcel.WriteString(host_)) {
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(
        std::min(DNS_RESULT_MAX_SIZE, static_cast<uint32_t>(addrlist_.size()))))) {
        return false;
    }
    uint32_t count = 0;
    for (const auto &addr : addrlist_) {
        if (!addr.Marshalling(parcel)) {
            return false;
        }
        if (++count >= DNS_RESULT_MAX_SIZE) {
            break;
        }
    }
    return true;
}


bool NetDnsResultReport::Unmarshalling(Parcel &parcel, NetDnsResultReport &resultReport)
{
    std::list<NetDnsResultAddrInfo>().swap(resultReport.addrlist_);

    if (!parcel.ReadUint32(resultReport.netid_) || !parcel.ReadUint32(resultReport.uid_) ||
        !parcel.ReadUint32(resultReport.pid_) || !parcel.ReadUint32(resultReport.timeused_)) {
        return false;
    }

    if (!parcel.ReadUint32(resultReport.queryresult_)) {
        return false;
    }

    if (!parcel.ReadString(resultReport.host_)) {
        return false;
    }

    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return false;
    }
    size = (size > DNS_RESULT_MAX_SIZE) ? DNS_RESULT_MAX_SIZE : size;
    for (uint32_t i = 0; i < size; ++i) {
        NetDnsResultAddrInfo addrInfo;
        if (!NetDnsResultAddrInfo::Unmarshalling(parcel, addrInfo)) {
            return false;
        }
        resultReport.addrlist_.push_back(addrInfo);
    }

    return true;
}
} // namespace NetsysNative
} // namespace OHOS
