/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_DNSRESOLV_H__
#define INCLUDE_DNSRESOLV_H__

#include <string>
#include <vector>

namespace OHOS {
namespace nmd {
const uint32_t MAX_NAME_LEN = 64;

struct DnsResParams {
    uint16_t baseTimeoutMsec;
    uint8_t retryCount = 1;
    void operator=(const DnsResParams &param)
    {
        baseTimeoutMsec = param.baseTimeoutMsec;
        retryCount = param.retryCount;
    }
};

struct DnsresolverParams {
    uint16_t netId = 0;
    uint16_t baseTimeoutMsec = 0;
    uint8_t retryCount = 0;
    std::vector<std::string> servers;
    std::vector<std::string> domains;
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_DNSRESOLV_H__
