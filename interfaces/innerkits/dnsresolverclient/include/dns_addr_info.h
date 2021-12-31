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

#ifndef DNS_ADDR_INFO_H
#define DNS_ADDR_INFO_H

#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {
struct DnsAddrInfo : public Parcelable {
    int32_t flags_ = 0;
    int32_t family_ = 0;
    int32_t sockType_ = 0;
    int32_t protocol_ = 0;
    std::string addr_;
    std::string canonName_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<DnsAddrInfo> Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif