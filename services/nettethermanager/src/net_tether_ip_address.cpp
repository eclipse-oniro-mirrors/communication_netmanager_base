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

#include "net_tether_ip_address.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherIpAddress::NetTetherIpAddress() : address_("0.0.0.0"), prefixLength_(0), ipType_(IP_TYPE::TYPE_UNKNOW) {}

NetTetherIpAddress::NetTetherIpAddress(const std::string &ipAddr, uint32_t prefixLength, bool isIpv4)
    : address_(ipAddr), prefixLength_(prefixLength)
{
    if (isIpv4) {
        ipType_ = IP_TYPE::TYPE_IPV4;
    } else {
        ipType_ = IP_TYPE::TYPE_IPV6;
    }
}

const std::string &NetTetherIpAddress::GetAddress() const
{
    return address_;
}

int32_t NetTetherIpAddress::GetPrefixLength() const
{
    return prefixLength_;
}

bool NetTetherIpAddress::IsIpv4() const
{
    return ipType_ == IP_TYPE::TYPE_IPV4;
}

bool NetTetherIpAddress::IsIpv6() const
{
    return ipType_ == IP_TYPE::TYPE_IPV6;
}

bool NetTetherIpAddress::InvalidAddr() const
{
    return ipType_ == IP_TYPE::TYPE_UNKNOW;
}
} // namespace NetManagerStandard
} // namespace OHOS