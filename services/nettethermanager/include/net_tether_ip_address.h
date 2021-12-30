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

#ifndef NET_TETHER_IP_ADDRESS_H
#define NET_TETHER_IP_ADDRESS_H

#include <string>

namespace OHOS {
namespace NetManagerStandard {
class NetTetherIpAddress {
public:
    enum IP_TYPE {
        TYPE_UNKNOW = -1,
        TYPE_IPV4 = 0,
        TYPE_IPV6 = 1
    };
    NetTetherIpAddress();
    NetTetherIpAddress(const std::string &ipAddr, uint32_t prefixLength, bool isIpv4);
    ~NetTetherIpAddress() = default;
    const std::string &GetAddress() const;
    int32_t GetPrefixLength() const;
    bool IsIpv4() const;
    bool IsIpv6() const;
    bool InvalidAddr() const;

private:
    std::string address_;
    uint32_t prefixLength_;
    IP_TYPE ipType_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_IP_ADDRESS_H