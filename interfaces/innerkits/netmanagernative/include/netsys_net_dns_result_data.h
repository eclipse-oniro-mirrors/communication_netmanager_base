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

#ifndef NETSYS_NET_DNS_RESULT_DATA_H
#define NETSYS_NET_DNS_RESULT_DATA_H

#include <iostream>
#include <list>

#include "parcel.h"

namespace OHOS {
namespace NetsysNative {
namespace {
constexpr uint32_t DNS_RESULT_MAX_SIZE = 32;
} // namespace

enum NetDnsResultAddrType : uint32_t {
    ADDR_TYPE_IPV4 = 0,
    ADDR_TYPE_IPV6 = 1,
};
#define NET_SYMBOL_VISIBLE __attribute__ ((visibility("default")))

struct NET_SYMBOL_VISIBLE NetDnsResultAddrInfo final : public Parcelable {
    uint32_t	    type_;
    std::string     addr_;

    bool Marshalling(Parcel &parcel) const override;
    static bool Unmarshalling(Parcel &parcel, NetDnsResultAddrInfo &addrinfo);
};

struct NET_SYMBOL_VISIBLE NetDnsResultReport final : public Parcelable {
    uint32_t        netid_;
    uint32_t        uid_;
    uint32_t        pid_;
    uint32_t        timeused_;
    uint32_t        queryresult_;
    std::string     host_;
    std::list<NetDnsResultAddrInfo> addrlist_;

    bool Marshalling(Parcel &parcel) const override;
    static bool Unmarshalling(Parcel &parcel, NetDnsResultReport &result);
};
} // namespace NetsysNative
} // namespace OHOS
#endif // NETSYS_NET_DNS_RESULT_DATA_H
