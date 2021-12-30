/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dns_addr_info.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool DnsAddrInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(flags_)) {
        return false;
    }
    if (!parcel.WriteInt32(family_)) {
        return false;
    }
    if (!parcel.WriteInt32(sockType_)) {
        return false;
    }
    if (!parcel.WriteInt32(protocol_)) {
        return false;
    }
    if (!parcel.WriteString(addr_)) {
        return false;
    }
    return true;
}

sptr<DnsAddrInfo> DnsAddrInfo::Unmarshalling(Parcel &parcel)
{
    sptr<DnsAddrInfo> ptr = (std::make_unique<DnsAddrInfo>()).release();
    if (ptr == nullptr) {
        NETMGR_LOG_E("create DnsAddrInfo failed");
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->flags_)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->family_)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->sockType_)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->protocol_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->addr_)) {
        return nullptr;
    }
    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS